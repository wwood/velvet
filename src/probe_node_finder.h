
IDnum* extract_best_probe_reads(Graph* graph, IDnum* probeReads, IDnum numProbeReads);


inline void extract_best_probe_reads_from_node_and_twin(
  Node* currentNode, Graph* graph, IDnum* probeReads, IDnum numProbeReads, IDnum* toReturn, ShortReadMarker* bestShortReads);
