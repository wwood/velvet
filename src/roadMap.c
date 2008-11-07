/*
Copyright 2007, 2008 Daniel Zerbino (zerbino@ebi.ac.uk)

    This file is part of Velvet.

    Velvet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Velvet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Velvet; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"
#include "graph.h"
#include "recycleBin.h"
#include "tightString.h"
#include "roadMap.h"

#ifndef NULL
#define NULL 0
#endif

union positionPtr {
	Coordinate coord;
	IDnum nodeID;
};

struct annotation_st {
	IDnum sequenceID;	// 32
	Coordinate position;	// 32
	union positionPtr start;	// 32
	union positionPtr finish;	// 32
	Coordinate length;	// 32
};

struct roadmap_st {
	IDnum annotationCount;
};

// Creates empty RoadMap
RoadMap *newRoadMap()
{
	return calloc(1, sizeof(RoadMap));
}

IDnum getAnnotationCount(RoadMap * rdmap)
{
	return rdmap->annotationCount;
}

Coordinate getFinish(Annotation * annot)
{
	return annot->finish.coord;
}

IDnum getAnnotSequenceID(Annotation * annot)
{
	return annot->sequenceID;
}

Coordinate getStart(Annotation * annot)
{
	return annot->start.coord;
}

Coordinate getPosition(Annotation * annot)
{
	return annot->position;
}

Coordinate getAnnotationLength(Annotation * annot)
{
	if (annot == NULL)
		return 0;

	return annot->length;
}

// Imports roadmap from the appropriate file format
// Memory allocated within the function
RoadMapArray *importRoadMapArray(char *filename)
{
	FILE *file;
	const int maxline = 100;
	char *line = malloc(sizeof(char) * maxline);
	RoadMap *array;
	RoadMap *rdmap = NULL;
	IDnum rdmapIndex = 0;
	IDnum seqID;
	Coordinate position, start, finish;
	Annotation *nextAnnotation;
	RoadMapArray *result = malloc(sizeof(RoadMapArray));
	IDnum sequenceCount;
	IDnum annotationCount = 0;

	if (line == NULL || result == NULL) {
		puts("Malloc failure");
		exit(1);
	}

	printf("Reading roadmap file %s\n", filename);

	file = fopen(filename, "r");
	fgets(line, maxline, file);
	sscanf(line, "%li\t%i\n", &sequenceCount, &(result->WORDLENGTH));
	result->length = sequenceCount;
	array = malloc(sequenceCount * sizeof(RoadMap));
	if (array == NULL && sequenceCount > 0) {
		puts("Malloc failure");
		exit(1);
	}
	result->array = array;

	while (fgets(line, maxline, file) != NULL)
		if (line[0] != 'R')
			annotationCount++;

	result->annotations = calloc(annotationCount, sizeof(Annotation));
	nextAnnotation = result->annotations;
	fclose(file);

	file = fopen(filename, "r");

	fgets(line, maxline, file);
	while (fgets(line, maxline, file) != NULL) {
		if (line[0] == 'R') {
			rdmap = getRoadMapInArray(result, rdmapIndex++);
			rdmap->annotationCount = 0;
		} else {
			sscanf(line, "%li\t%li\t%li\t%li\n", &seqID,
			       &position, &start, &finish);
			nextAnnotation->sequenceID = seqID;
			nextAnnotation->position = position;
			nextAnnotation->start.coord = start;
			nextAnnotation->finish.coord = finish;

			if (seqID > 0)
				nextAnnotation->length = finish - start;
			else
				nextAnnotation->length = start - finish;


			rdmap->annotationCount++;
			nextAnnotation++;
		}
	}

	printf("%li roadmaps reads\n", rdmapIndex);

	fclose(file);
	free(line);
	return result;
}

RoadMap *getRoadMapInArray(RoadMapArray * array, IDnum index)
{
	return &(array->array[index]);
}

void setStartID(Annotation * annot, IDnum nodeID)
{
	annot->start.nodeID = nodeID;
}

void setFinishID(Annotation * annot, IDnum nodeID)
{
	annot->finish.nodeID = nodeID;
}

IDnum getStartID(Annotation * annot)
{
	return annot->start.nodeID;
}

IDnum getFinishID(Annotation * annot)
{
	return annot->finish.nodeID;
}

void incrementAnnotationCoordinates(Annotation * annot)
{
	annot->start.coord++;
	annot->finish.coord++;
}

Annotation *getNextAnnotation(Annotation * annot)
{
	return annot + 1;
}
