#ifndef _READ_TO_NODE_H_
#define _READ_TO_NODE_H_

typedef struct {
  IDnum read_id;
  IDnum node_id;
} ReadIdNodeId; //this is a doubly used structure, once for creation of the data structure, and also in the result of the indexation. This is a little bad, but eh.

typedef struct {
  IDnum num_contents;
  IDnum num_reads;
  IDnum* index;
  ReadIdNodeId* contents;
} ReadIdToNodeIdLookupTable;

//TODO: add 'writing is complete' flag like other velvet functions
typedef struct {
  IDnum num_contents;
  IDnum num_reads;
} ReadIdToNodeIdLookupTableFileHeader;

typedef struct {
  IDnum num_nodes;
  ReadIdNodeId* read_ids_node_ids;
} ReadIdToNodeIdIndexation;

ReadIdToNodeIdLookupTable* createReadToNode(Graph* graph);
void destroyReadIdToNodeIdLookupTable(ReadIdToNodeIdLookupTable* lookupTable);

ReadIdToNodeIdIndexation getReadIdToNodeIdIndexation(ReadIdToNodeIdLookupTable* lookupTable, IDnum readID);

void writeReadIdToNodeIdLookupTable(char* fileName, ReadIdToNodeIdLookupTable* lookupTable);
ReadIdToNodeIdLookupTable* readReadIdToNodeIdLookupTable(char* fileName);

#endif
