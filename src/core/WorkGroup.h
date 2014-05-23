// WorkGroup.h (Oclgrind)
// Copyright (c) 2013-2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include "common.h"

#define CLK_LOCAL_MEM_FENCE  (1<<0)
#define CLK_GLOBAL_MEM_FENCE (1<<1)

namespace oclgrind
{
  class Device;
  class Memory;
  class Kernel;
  class WorkItem;

  class WorkGroup
  {
  public:
    enum AsyncCopyType{GLOBAL_TO_LOCAL, LOCAL_TO_GLOBAL};

  private:
    // Comparator for ordering work-items
    struct WorkItemCmp
    {
      bool operator()(const WorkItem *lhs, const WorkItem *rhs) const;
    };
    std::set<WorkItem*, WorkItemCmp> m_running;

    typedef struct
    {
      const llvm::Instruction *instruction;
      AsyncCopyType type;
      size_t dest;
      size_t src;
      size_t size;
      size_t num;
      size_t srcStride;
      size_t destStride;

      size_t event;
    } AsyncCopy;

    typedef struct
    {
      const llvm::Instruction *instruction;
      std::set<WorkItem*, WorkItemCmp> workItems;

      uint64_t fence;
      std::list<size_t> events;
    } Barrier;

  public:
    WorkGroup(Device *device, const Kernel& kernel, Memory &globalMem,
              unsigned int workDim,
              size_t wgid_x, size_t wgid_y, size_t wgid_z,
              const size_t globalOffset[3],
              const size_t globalSize[3],
              const size_t wgsize[3]);
    virtual ~WorkGroup();

    size_t async_copy(
      const WorkItem *workItem,
      const llvm::Instruction *instruction,
      AsyncCopyType type,
      size_t dest,
      size_t src,
      size_t size,
      size_t num,
      size_t srcStride,
      size_t destStride,
      size_t event);
    void clearBarrier();
    const size_t* getGlobalOffset() const;
    const size_t* getGlobalSize() const;
    const size_t* getGroupID() const;
    const size_t getGroupIndex() const;
    const size_t* getGroupSize() const;
    Memory* getLocalMemory() const;
    WorkItem *getNextWorkItem() const;
    unsigned int getWorkDim() const;
    WorkItem *getWorkItem(size_t localID[3]) const;
    bool hasBarrier() const;
    void notifyBarrier(WorkItem *workItem, const llvm::Instruction *instruction,
                       uint64_t fence,
                       std::list<size_t> events=std::list<size_t>());
    void notifyFinished(WorkItem *workItem);

  private:
    unsigned int m_workDim;
    size_t m_groupIndex;
    size_t m_globalOffset[3];
    size_t m_globalSize[3];
    size_t m_groupID[3];
    size_t m_groupSize[3];
    Device *m_device;
    const Kernel& m_kernel;
    Memory *m_localMemory;
    std::vector<WorkItem*> m_workItems;

    Barrier *m_barrier;
    size_t m_nextEvent;
    std::list< std::pair<AsyncCopy,std::set<const WorkItem*> > > m_asyncCopies;
    std::map < size_t, std::list<AsyncCopy> > m_events;
  };
}