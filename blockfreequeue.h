#ifndef BLOCKFREEQUEUE_H
#define BLOCKFREEQUEUE_H


#include<atomic>
#include<memory>
#include<queue>

#include "threadfreecout.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Block free thread safe queue
///
/// Not intended for high-load applications!!!
/// freeing poped nodes may stall if to many processes do push/pop same time
/// wich will lead to overuse of memory
///
/// can lead to memory leaks if destructor has finished before operations have complited
/// (i.e. be carefull to the order of initialization)
///
template<typename T>
class BlockFreeQueue
{
    //-------------------------------------------------------------------------------------------
    struct Node{
        std::shared_ptr<T> pdata;
        std::atomic<Node *> next;
        Node():pdata{std::make_shared<T>()},next{nullptr}{};
        Node(const T &data_):pdata{std::make_shared<T>(data_)},next{nullptr}{};
        ~Node(){
        }

    };
    //-------------------------------------------------------------------------------------------
    // exception safe cover for in/out check of deletion procedures
    struct InPushFlag{
        std::atomic<int>* const pDelSec;
        InPushFlag(std::atomic<int>* const pDelSec_):pDelSec(pDelSec_){; (*pDelSec)++;};
        ~InPushFlag(){(*pDelSec)--;};
    };
    //-------------------------------------------------------------------------------------------
    std::atomic<Node*> head;                //  main queue body
    Node* tail;                             //  main queue tail
    std::atomic<Node*> delete_queue;        //  queue for nodes to delete
    std::atomic<int> inDeleteSec;           //  flag for sinc of deletion procedures
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief thread safe deleting of nodes
    /// \param oldnode - node pair to delete. Must be pair of Node(Data)-Node(nullptr)
    /// if there are more then pair of nodes, behavor is undefined
    ///
    void tryToDelete(Node * oldnode){
        // algorithm:
        // 1. push_front node pair to delete_queue
        // 2. store old head of delete_queue to local variable because we don't now if  delete_queue has been incremented;
        // 3. check if we are alone (i.e. inDeleteSec == 1): that garanty, thera are no other pointers to node to delete in other processes for now
        // 4. cut tail to delete (atomic exchange head of delete_queue to nullptr with delque),
        //    and if delete_queue has already run forward (!=delque) repeat
        //    and if run forward then check inDeleteSec == 1 to garanty other process free pointers to node
        //    memory sequence garanty delque point to begin of delqueue, other process free ponters to nodes and since delqueue set nothing has changed
        // 5. usual delete of nodes from cut tail

        // 1. push_front node pair to delete_queue
        for (;;){
            oldnode->next.load()->next.store(delete_queue);
            Node * oldnodenext = oldnode->next.load()->next.load();
            //
            if (delete_queue.compare_exchange_weak(oldnodenext,oldnode)){
                break;
            }
        }

        //2. store old head of delete_queue to local variable
        Node* delque = oldnode;

        // 3. check if we are alone (i.e. inDeleteSec == 1)
        if(inDeleteSec.load() == 1){

            // 4. cut tail to delete
            while (!delete_queue.compare_exchange_weak(delque,nullptr)) {
                // to provide sequence [we have head of the queue]-[only this thread has pointer]
                if(inDeleteSec.load() != 1){
                    return;
                }
            }

            // 5. usual delete of nodes from cut tail
            Node* dTmp;
            while(delque != nullptr){
                dTmp = delque->next;
                delete delque;
                delque = dTmp;
            }
        }
    };
        //-------------------------------------------------------------------------------------------
public:
    //-------------------------------------------------------------------------------------------
    BlockFreeQueue():head{nullptr},delete_queue{nullptr},inDeleteSec{0}{
        head = new Node();
        tail = head;
        ;};
    //-------------------------------------------------------------------------------------------
    virtual ~BlockFreeQueue(){
        clear();
        // TODO: if no crashing -- redo
        delete head;
        {
            ThreadFreeCout pcout;
            pcout <<"~BlockFreeQueue()\n";
        }
    };
    //-------------------------------------------------------------------------------------------
    // to simplify drop copy constructors
    BlockFreeQueue(BlockFreeQueue&) =delete;
    BlockFreeQueue(const BlockFreeQueue&) =delete;
    BlockFreeQueue& operator=(const BlockFreeQueue&) =delete;
    //-------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief thread safe Push data to queue
    /// \param data
    ///
    void Push(T & data){
        InPushFlag flg(&inDeleteSec);

        Node* new_node = new Node(data);
        Node* new_tail = new Node();
        new_node->next.store(new_tail);

        for(;;){
            Node* oldnext = nullptr;
            if(tail->next.compare_exchange_weak(oldnext,new_node)) {
                tail = new_tail;
                break;
            };
        }
    };
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief thread safe Push data to queue
    /// \param data (i.e. rvalue)
    ///
    void Push(T&& data){
        InPushFlag flg(&inDeleteSec);

        Node* new_node = new Node(std::move(data));
        Node* new_tail = new Node();
        new_node->next.store(new_tail);

        for(;;){
            Node* oldnext = nullptr;
            if(tail->next.compare_exchange_weak(oldnext,new_node)) {
                tail = new_tail;
                break;
            };
        }
    };
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief thread safe pop data from stack. if no data set bSuccess to false
    /// \param if no data set bSuccess to false
    /// \return shared_ptr<T> to data
    ///
    std::shared_ptr<T> Pop(bool & bSuccess){
        InPushFlag flg(&inDeleteSec);
        //
        bSuccess = false;
        std::shared_ptr<T> pRet;
        //

        Node* old_node = head;
        Node* next_node = old_node->next.load();

        for(;;){
            old_node = head;
            if (old_node->next == nullptr){
                break;
            }
            next_node = old_node->next.load();

            if(head.compare_exchange_weak(old_node,next_node->next)){
                pRet = move(old_node->next.load()->pdata);
                bSuccess = true;
                tryToDelete(old_node);
                break;
            }
        }
        return pRet;
    };
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief thread safe clearing stack data
    ///
    void clear(){
        InPushFlag flg(&inDeleteSec);
        //
        Node* old_node = head;
        Node* next_node = old_node->next.load();

        for(;;){
            old_node = head;
            if (old_node->next == nullptr){
                break;
            }
            next_node = old_node->next.load();

            if(head.compare_exchange_weak(old_node,next_node->next)){
                tryToDelete(old_node);
            }
        }
    };
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get length of queue. Q(N) time complexity. To find out of queue is free use ::empty()
    /// \return
    ///
    size_t size()
    {
        InPushFlag flg(&inDeleteSec);
        size_t iCount{0};

        std::atomic<Node*> cur_node = head.load();
        Node* old_node;
        Node* next_node;

        for(;;){
            old_node = cur_node;
            if (old_node->next == nullptr){
                break;
            }
            next_node = old_node->next.load();
            if(cur_node.compare_exchange_weak(old_node,next_node->next)){
                iCount++;
            }
        }
        return iCount;
    }
    //-------------------------------------------------------------------------------------------
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Return true if queue is empty
    /// \return
    ///
    bool empty(){
        Node* cur_node = head.load();
        if (cur_node->next == nullptr){
            return true;
        };
        return false;
    }
    //-------------------------------------------------------------------------------------------
};

#endif // BLOCKFREEQUEUE_H
