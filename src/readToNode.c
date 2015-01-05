#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "utility.h"
#include "readToNode.h"
#include "graphStructures.h"
#include "graph.h"

// comparison function for qsort. Sort ascending, first by read ID then by node ID
static int read_id_node_id_compare_function(const void *p1, const void *p2){
  ReadIdNodeId* rn1 = (ReadIdNodeId*) p1;
  ReadIdNodeId* rn2 = (ReadIdNodeId*) p2;
  if (rn1->read_id < rn2->read_id)
    return -1;
  else if (rn1->read_id > rn2->read_id)
    return 1;
    else return (rn1->node_id - rn2->node_id);
}

/** Create a new ReadIdToNodeIdHash structure with all the given
information in the graph passed here.
*/
ReadIdToNodeIdLookupTable* createReadToNode(Graph* graph){
  // malloc array of read_id_node_id structs that is 1/10 the size of the number of nodes,
  // or 100, whichever is larger (to account for if the number of nodes < 10
  IDnum reads_and_nodes_length = graph->nodeCount / 10;
  if (reads_and_nodes_length < 100)
    reads_and_nodes_length = 100;
  ReadIdNodeId* reads_and_nodes = mallocOrExit(reads_and_nodes_length, ReadIdNodeId);

  IDnum reads_and_nodes_count = 0;
  IDnum reads_and_nodes_step_size = reads_and_nodes_length;

  // write reads_and_nodes array unsorted
  // For each node in order
  IDnum i, j, k;
  // to prevent code duplication, create an array of 2 node pointers
  Node** fwd_and_rev_nodes = mallocOrExit(2, Node*);
  Node* current_node;
  ShortReadMarker current_read;
  IDnum num_short_reads_attached;
  ShortReadMarker* current_node_reads;
  for (i=0; i<graph->nodeCount; i++){
    current_node = getNodeInGraph(graph, i+1);
    if (getNodeID(current_node) < 0)
      continue; //ignore reverse nodes as these are already taken care of

    fwd_and_rev_nodes[0] = current_node;
    fwd_and_rev_nodes[1] = getTwinNode(current_node);
    for (j=0; j<2; j++){
      current_node = fwd_and_rev_nodes[j];
      num_short_reads_attached = getNodeReadCount(current_node, graph);
      current_node_reads = getNodeReads(current_node, graph);

      for (k=0; k<num_short_reads_attached; k++){
        // For each fwd facing short read attached to the node
        // if we are beyond capacity, realloc some more
        if (reads_and_nodes_count >= reads_and_nodes_length){
          reads_and_nodes_length += reads_and_nodes_step_size;
          //printf("Re-allocing to size %i\n", reads_and_nodes_length);
          reads_and_nodes = reallocOrExit(reads_and_nodes, reads_and_nodes_length, ReadIdNodeId);
        }

        // add the current read
        current_read = current_node_reads[k];
        //printf("Accessing index %i\n", reads_and_nodes_count);
        ReadIdNodeId* rid_nid = &(reads_and_nodes[reads_and_nodes_count]);
        rid_nid->read_id = current_read.readID;
        rid_nid->node_id = current_node->ID;

        //increment the counter
        reads_and_nodes_count += 1;
      }
    }
  }
  free(fwd_and_rev_nodes);

  //realloc down to the final size
  reads_and_nodes_length = reads_and_nodes_count;
  reads_and_nodes = reallocOrExit(reads_and_nodes, reads_and_nodes_length, ReadIdNodeId);

  //quicksort the struct array, sorting only by read ID
  qsort(reads_and_nodes, reads_and_nodes_length, sizeof(ReadIdNodeId), read_id_node_id_compare_function);

  // malloc an index array of read id to reads_and_nodes offset the same length as the total number of reads
  // the largest read ID recorded is the read ID of the largest (now sorted) reads_and_nodes
  IDnum num_reads = reads_and_nodes[reads_and_nodes_length-1].read_id;
  IDnum* index_array = callocOrExit(num_reads, IDnum);

  // with sorted read_id_node_id array, write out the index array
  IDnum last_read_id = 0;
  for (i=0; i<reads_and_nodes_length; i++){
    //if the last node id is different to the current node ID, write the new one to the index
    if (reads_and_nodes[i].read_id != last_read_id ){
      index_array[reads_and_nodes[i].read_id-1] = i;
      last_read_id = reads_and_nodes[i].read_id;
    }
  }

  ReadIdToNodeIdLookupTable* lookupTable = mallocOrExit(1, ReadIdToNodeIdLookupTable);
  lookupTable->num_contents = reads_and_nodes_length;
  lookupTable->num_reads = num_reads;
  lookupTable->contents = reads_and_nodes;
  lookupTable->index = index_array;
  return lookupTable;
}



//write structure to file
void writeReadIdToNodeIdLookupTable(char* fileName, ReadIdToNodeIdLookupTable* lookupTable){
  FILE* fp = fopen(fileName, "wb");
  if (fp == NULL)
    velvetLog("Unable to open %s for reading\n", fileName);

  //write overall struct info
  ReadIdToNodeIdLookupTableFileHeader header;
  header.num_contents = lookupTable->num_contents;
  header.num_reads = lookupTable->num_reads;
  fwrite(&header, sizeof(ReadIdToNodeIdLookupTableFileHeader), 1, fp);

  //write index array
  fwrite(lookupTable->index, sizeof(IDnum), lookupTable->num_reads, fp);

  //write contents array
  fwrite(lookupTable->contents, sizeof(ReadIdNodeId), lookupTable->num_contents, fp);

  //close up shop
  fclose(fp);
}

//read structure from file
ReadIdToNodeIdLookupTable* readReadIdToNodeIdLookupTable(char* fileName){
  //open file
  FILE* fp = fopen(fileName, "rb");
  if (fp == NULL)
    velvetLog("Unable to open %s for reading\n", fileName);

  ReadIdToNodeIdLookupTable* lookupTable = mallocOrExit(1, ReadIdToNodeIdLookupTable);

  // read header
  ReadIdToNodeIdLookupTableFileHeader header;
  if (fread(&header, sizeof(header), 1, fp) != 1){
		velvetLog("Unable to read file %s\n", fileName);
		fclose(fp);
		return NULL;
  }
  lookupTable->num_contents = header.num_contents;
  lookupTable->num_reads = header.num_reads;

  //read index array
  lookupTable->index = mallocOrExit((lookupTable->num_reads), IDnum);
  if (fread(lookupTable->index, sizeof(IDnum), lookupTable->num_reads, fp) != lookupTable->num_reads){
		velvetLog("Unknown format (at index stage) for file %s\n", fileName);
		fclose(fp);
		return NULL;
  }

  //read contents array
  lookupTable->contents = mallocOrExit(lookupTable->num_contents, ReadIdNodeId);
  if (fread(lookupTable->contents, sizeof(ReadIdNodeId), lookupTable->num_contents, fp) != lookupTable->num_contents){
		velvetLog("Unknown format (at contents stage) for file %s\n", fileName);
		fclose(fp);
		return NULL;
  }

  //close up shop
  fclose(fp);

  return lookupTable;
}

/* The main 'getter' function here. Return a struct representing an array start pointer and number of nodes
(array entries) that the read is attached to.
*/
ReadIdToNodeIdIndexation getReadIdToNodeIdIndexation(ReadIdToNodeIdLookupTable* lookupTable, IDnum readID){
  ReadIdToNodeIdIndexation toReturn;
  toReturn.read_ids_node_ids = lookupTable->contents + lookupTable->index[readID-1]; //minus one as the array is 0-indexed

  // work out the number of nodes by using the next read's offset, or
  if (readID == lookupTable->num_reads){
    toReturn.num_nodes = lookupTable->index[lookupTable->num_contents] - lookupTable->index[readID-1];
  } else {
    toReturn.num_nodes = lookupTable->index[readID] - lookupTable->index[readID-1];
  }

  return toReturn;
}


void destroyReadIdToNodeIdLookupTable(ReadIdToNodeIdLookupTable* lookupTable){
  free(lookupTable->index);
  free(lookupTable->contents);
  free(lookupTable);
}



// int main(int argc, char **argv){
//   Graph* graph = importGraph(argv[1]);
//   ReadIdToNodeIdLookupTable* readToNode = createReadToNode(graph);
//   printf("Parsed in graph with  %i reads and %i entries\n", readToNode->num_reads, readToNode->num_contents);
//   printf("First entry was read %i and node %i\n", readToNode->contents[0].read_id, readToNode->contents[0].node_id);
//   printf("2nd entry was read %i and node %i\n", readToNode->contents[1].read_id, readToNode->contents[1].node_id);
//   char* fileName = "some.readToNodeTable";
//   writeReadIdToNodeIdLookupTable(fileName, readToNode);
//   printf("Finished writing\n");
//   ReadIdToNodeIdLookupTable* readToNode2 = readReadIdToNodeIdLookupTable(fileName);
//   printf("Finished reading\n");
//   printf("Found %i reads and %i entries\n\n", readToNode2->num_reads, readToNode2->num_contents);

//   ReadIdToNodeIdIndexation res;
//   res = getReadIdToNodeIdIndexation(readToNode, 1);
//   printf("Found %i (len %i) for that read 1, expected 1, len 1\n", res.read_ids_node_ids[0].node_id, res.num_nodes);
//   res = getReadIdToNodeIdIndexation(readToNode, 2);
//   printf("Found %i (len %i) for that read 2, expected -1, len 1\n", res.read_ids_node_ids[0].node_id, res.num_nodes);
//   res = getReadIdToNodeIdIndexation(readToNode, 11);
//   printf("Found %i (and %i) (len %i) for that read 11, expected [4], len 1\n", res.read_ids_node_ids[0].node_id, res.num_nodes);
//   res = getReadIdToNodeIdIndexation(readToNode, 12);
//   printf("Found %i and %i (len %i) for that read 12, expected [5,6], len 2\n", res.read_ids_node_ids[0].node_id, res.read_ids_node_ids[1].node_id, res.num_nodes);
//   res = getReadIdToNodeIdIndexation(readToNode, 13);
//   printf("Found %i (len %i) for that read 13, expected -1, len 1\n", res.read_ids_node_ids[0].node_id, res.num_nodes);

//   res = getReadIdToNodeIdIndexation(readToNode, 3452);
//   printf("Found %i and %i (len %i) for that read 3452\n", res.read_ids_node_ids[0].node_id, res.read_ids_node_ids[1].node_id, res.num_nodes);
//   res = getReadIdToNodeIdIndexation(readToNode2, 3452);
//   printf("Found %i and %i (len %i) for that read 3452\n", res.read_ids_node_ids[0].node_id, res.read_ids_node_ids[1].node_id, res.num_nodes);

//   destroyReadIdToNodeIdLookupTable(readToNode);
//   destroyReadIdToNodeIdLookupTable(readToNode2);
//   destroyGraph(graph);

//   return 0;
// }
