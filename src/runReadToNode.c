#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "utility.h"
#include "readToNode.h"
#include "graphStructures.h"
#include "graph.h"

int main(int argc, char **argv){
  if (argc != 2){
    printf("Usage: readToNode <lastgraph_file>\n");
    exit(1);
  }

  Graph* graph = importGraph(argv[1]);
  ReadIdToNodeIdLookupTable* readToNode = createReadToNode(graph);
  printf("Parsed in graph with  %i reads and %i entries\n", readToNode->num_reads, readToNode->num_contents);

  char* fileName = "ReadToNode.bin";
  writeReadIdToNodeIdLookupTable(fileName, readToNode);
  printf("Finished writing\n");

  destroyReadIdToNodeIdLookupTable(readToNode);
  destroyGraph(graph);

  return 0;
}
