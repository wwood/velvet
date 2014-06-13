
#include <stdlib.h>

#include "globals.h"
#include "graph.h"
#include "graphStructures.h"
#include "utility.h"

#include "probe_node_finder.h"



/* Given an array of probe read IDs, return an array of pointers to nodes
that that are the best probe_read_ids.

graph: velvet graph_st
probeReads: array of probe read IDs
numProbeReads: number of elements in probeReads
*/
IDnum* extract_best_probe_reads(Graph* graph, IDnum* probeReads, IDnum numProbeReads){
  //malloc+zero result array - one IDnum for each probe given
  IDnum* toReturn = mallocOrExit(numProbeReads, IDnum);
  IDnum i;
  for (i=0; i<numProbeReads; i++){
    toReturn[i] = 0;
  }
  //initialise the corresponding set of best short reads
  ShortReadMarker* bestShortReads = mallocOrExit(numProbeReads, ShortReadMarker);

  // iterate over all nodes in the graph
  Node* currentNode;
  for (i = 1; i <= graph->nodeCount; i++){
    currentNode = graph->nodes[i];

    //find from the fwd node
    extract_best_probe_reads_from_node_and_twin(currentNode, graph, probeReads, numProbeReads, toReturn, bestShortReads);

    //find from the twin node
    extract_best_probe_reads_from_node_and_twin(getTwinNode(currentNode), graph, probeReads, numProbeReads, toReturn, bestShortReads);
  }
  //printf("found %d %d\n",toReturn[0],toReturn[1]);

  //clean up
  free(bestShortReads);

  //return nodes
  return toReturn;
}


/* the internals of a for loop in extract_best_probe_reads called
once each for fwd and twin nodes */
inline void extract_best_probe_reads_from_node_and_twin(
  Node* currentNode, Graph* graph, IDnum* probeReads, IDnum numProbeReads, IDnum* toReturn, ShortReadMarker* bestShortReads){

  IDnum j, k;
  ShortReadMarker* currentNodeShortReads;
  ShortReadMarker currentShortRead;
  IDnum currentNodeShortReadCount;

  currentNodeShortReadCount = getNodeReadCount(currentNode, graph);
  currentNodeShortReads = getNodeReads(currentNode, graph);

  for (j=0; j < currentNodeShortReadCount; j++){
    currentShortRead = currentNodeShortReads[j];
    //iterate over the given reads
    for (k=0; k<numProbeReads; k++){
      //printf("testing node %d, %d/%d %d/%d\n", getNodeID(currentNode), j,currentShortRead.readID, k,probeReads[k]);
      if (currentShortRead.readID == probeReads[k]){
        //printf("acceptance of node %d, %d/%d %d/%d\n", getNodeID(currentNode), j,currentShortRead.readID, k,probeReads[k]);
        //possible probe read found. But is it the best one?
        if (toReturn[k] == 0 ||
            bestShortReads[k].offset > currentShortRead.offset){
          // this is the first time this probe read has been found.
          // so it is the best.
          // or, a better one was just found
          //printf("second acceptance..\n");
          toReturn[k] = currentNode->ID;
          bestShortReads[k] = currentShortRead;
        }
      }
    }
  }
}
