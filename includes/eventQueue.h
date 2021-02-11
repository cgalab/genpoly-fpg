/* 
 * Copyright 2021 Philipp Mayer - pmayer@cs.sbg.ac.at
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

/*
	Include standard libraries
*/
#include <stdlib.h>

/*
	Include my headers	
*/
#include "triangle.h"
#include "vertex.h"
#include "tedge.h"
#include "settings.h"

/*
	Define the class EventQueue:
	The EventQueue is basically a list of events which contains all at the moment known
	future events ordered by their event times. An instance of EventQueue always belongs
	to one translation and will be deleted after the translation will have finished. It
	is capable of checking the numerical stability of a translation by checking (and
	changing) the order of neighboring events and aborting the translation if instability
	is expected.
*/
#ifndef __EVENTQUEUE_H_
#define __EVENTQUEUE_H_

/*
	The struct Event is the type of elements in the EventQueue. It consists of the
	following information:
	collapseTime 	The time at which the event will occur
	triangle 		The triangle which will collapse at the event
	next 			A pointer to the next event
*/
struct Event{
	double collapseTime;
	Triangle *triangle;

	struct Event *next;
};

class EventQueue{

private:
	
	/*
		A pointer to the first (earliest) event in the event queue.
		Has to be NULL as long as the event queue is empty
	*/
	struct Event *first;

	/*
		The actual number of events in the event queue
	*/
	int n;


	/*
		Vertices of the recent translation
	*/

	/*
		The real moving vertex
	*/
	Vertex * const original;

	/*
		A copy at the start position of the translation
	*/
	Vertex * const oldV;

	/*
		A copy at the target position of the translation
	*/
	Vertex * const newV;


	/*
		P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
	*/

	/*
		The function stabilize() checks and changes (if necessary) the ordering of two
		neighboring events in the event queue with very close event times. If the
		corresponding triangles of the events are adjacent it decides whether the opposite
		edges to the original vertex form a convex or a non-convex shape and call
		stabilizeConvex() resp. stabilizeNonConvex() to check the ordering of the events.
		In case the corresponding triangles are not adjacent, the exact order does not
		matter, so stabilize() does nothing.

		@param 	e0 	The event which is scheduled earlier in the event queue
		@param 	e1 	The event which is scheduled later in the event queue

		Note:
			- It is required that the collapseTime of e0 is less than the collapseTime of e1
			- In theory this should work, but in practice it does not at the moment
	*/
	void stabilize(struct Event *e0, struct Event *e1);

	/*
		The function stabilizeConvex() checks for two events with adjacent triangles and
		convex shape of the opposing edges whether the events are in the right order and
		change if necessary. In case the events are in the wrong order the events
		themselves are not changed, just the triangles are switched such that it still
		holds that the events in the event queue are order by their collapse times.
		For further information on the decision criterion for reordering take a look into
		my master thesis.

		@param 	e0 			The event which is scheduled earlier in the event queue
		@param 	e1 			The event which is scheduled later in the event queue
		@param 	commonE 	The edge which is shared by the triangles of e0 and e1

		Note:
			It is important not to change the events, but just switch their triangles,
			otherwise this might lead to an infinite loop in makeStable().	
	*/
	void stabilizeConvex(struct Event *e0, struct Event *e1, TEdge *commonE);

	/*
		The function stabilizeNonConvex() checks for two events with adjacent triangles
		and non-convex shape of the opposing edges whether the events are in the right
		order and change if necessary. In case the events are in the wrong order the
		events themselves are not changed, just the triangles are switched such that it
		still holds that the events in the event queue are order by their collapse times.
		For further information on the decision criterion for reordering take a look into
		my master thesis.

		@param 	e0 			The event which is scheduled earlier in the event queue
		@param 	e1 			The event which is scheduled later in the event queue
		@param 	commonE 	The edge which is shared by the triangles of e0 and e1

		Note:
			It is important not to change the events, but just switch their triangles,
			otherwise this might lead to an infinite loop in makeStable()	
	*/
	void stabilizeNonConvex(struct Event *e0, struct Event *e1, TEdge *commonE);

public:

	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:
		
										EventQueue(Vertex * const orig, Vertex * const oV,
										Vertex * const nV) 

		OTHERS:

		void 							insertWithoutCheck(const double time, Triangle *t)
		bool 							makeStable(const bool initial)
		std::pair<double, Triangle*> 	pop()
		int 							size() const
		void 							remove(Triangle * const t)
		void 							print() const
	*/


	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:

		@param 	orig 	The vertex which is moved by the translation
		@param 	oV 		A copy of the vertex at its start position
		@param	nV 		A copy of the vertex at its target position
	*/	
	EventQueue(Vertex * const orig, Vertex * const oV, Vertex * const nV);


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function insertWithoutCheck() creates a new event for the triangle t collapsing
		at time and inserts it into the list of events such that the list is still order by
		the collapse times of the events.

		@param 	time 	The collapse time for the new event
		@param 	t 		The triangle which will collapse at the new event

		Note:
			The ordering of the events will not be checked by this function!
	*/
	void insertWithoutCheck(const double time, Triangle *t);

	/*
		The function makeStable() iterates over the whole event queue and searches for
		events closer then Settings::epsEventTime which are then considered as concurrent.
		Two concurrent events then get checked and potentially reorder by the function
		stabilize(). If it finds three or more neighboring events which are pairwise
		concurrent, it returns false and the translation will be aborted.

		@param 	initial 	Indicates whether the event queue is the initial event queue
							or not, which leads to different outputs in case the
							translation gets aborted.
		@return 			True if there are not more than two neighboring concurrent
							events, otherwise false

		Note:
			- Due to stability issues this function is unused at the moment.
	*/
	bool makeStable(const bool initial);

	/*
		The function pop() returns the element on top of the event queue and removes it.

		@return 	The element on top of the event queue
	*/
	std::pair<double, Triangle*> pop();

	/*
		@return 	The actual number of events in the event queue
	*/
	int size() const;

	/*
		The function remove() searches for an event containing the triangle t in the event
		queue and deletes it. If no event with this triangle exists in the event queue it
		simple does nothing.

		@param 	t 	The triangle to be deleted
	*/
	void remove(Triangle * const t);
	
	/*
		The function print() prints all event times in an ordered way to stdout.
	*/
	void print() const;


	/*
		D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
	*/

	/*
		Destructor:
		The destructor frees up all memory of still existing events and sets the enqueued
		flag of the remaining triangles back to false.

		Note:
			If the translation is aborted the event queue might not be empty in the end, so
			it is necessary to use the destructor.
	*/
	~EventQueue();
};

#endif
