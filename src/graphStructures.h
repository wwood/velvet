struct arc_st {
	Arc *twinArc;		// 64
	Arc *next;		// 64
	Arc *previous;		// 64
	Arc *nextInLookupTable;	// 64
	Node *destination;	// 64
	IDnum multiplicity;	// 32
}  ATTRIBUTE_PACKED;	// 352 Total

struct node_st {
	Node *twinNode;		// 64
	Arc *arc;		// 64
	Descriptor *descriptor;	// 64
	PassageMarkerI marker;	// 32
	IDnum length;	// 32
#ifndef SINGLE_COV_CAT
	IDnum virtualCoverage[CATEGORIES];	// 32 * 2
	IDnum originalVirtualCoverage[CATEGORIES];	// 32 * 2
#else
	IDnum virtualCoverage;	// 32 * 2
#endif
	IDnum ID;		// 32
	IDnum arcCount;		// 32
	boolean status;		// 1
	boolean uniqueness;	// 1
} ATTRIBUTE_PACKED;	// 418 Total

struct shortReadMarker_st {
	IDnum position;
	IDnum readID;
	ShortLength offset;
} ATTRIBUTE_PACKED;

struct gapMarker_st {
	GapMarker *next;
	IDnum position;
	IDnum length;
} ATTRIBUTE_PACKED;

struct graph_st {
	Node **nodes;
	Arc **arcLookupTable;
	ShortReadMarker **nodeReads;
	IDnum *nodeReadCounts;
	GapMarker **gapMarkers;
	Coordinate insertLengths[CATEGORIES + 1];
	double insertLengths_var[CATEGORIES + 1];
	IDnum sequenceCount;
	IDnum nodeCount;
	int wordLength;
	boolean double_stranded;
};