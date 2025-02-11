#ifndef _HALFEDGE
#define _HALFEDGE
#endif

/*	This header file stores definition, functions and iterators  ..
..	related to ..	a half edge " h " and full edge " H ". A half  ..
..	edge,h := (v, v'), is a directed edge made from two vertices ..
..	" v " and " v' ". The "complement" or "flip" or "reverse"    ..
..	half edge is the  halfe edge " h' " defined as h' = (v',v).  ..
..	A full edge or the frontedge (as in this headerfile ) is the ..
..	set H = {h,h'}. An oriented triangle is comprised of         ..
..	cyclically ordered halfedges F = {h0,h1,h2}.                 ..
..	Later: Remove many assert(); */
typedef struct _Hedge Hedge;

struct _Hedge{
	/*  Definition of a halfedge h. "H" stores to which full edge ..
	..  this half edge belongs to. "flags" stores (i) if this half..
	..  edge is h or h' in H. flags&4==0 means first half edgea & ..
	..  flags&4 == 4  means it is the second half edge of H       ..
	..  (ii) what is the order of hedge in the oriented polygon.  ..
	..  Face order is defined by (flag&24)>>3 in {0,1,2,3}.       ..
	..	(iii) Regrid flag: flags&3 : 0-donothing/1-delete/2-split ..
	..	"v" is the pivot vertex of halfedge h:=(v,v').            ..
	*/
	Hedge * next, * prev, * flip;
	Frontpoint * v;

	long alias,id;
	int pid;	
	unsigned short flags; 
};      

enum HFLAGS {
  HMESH = 0,
  /* bit shift */
  HMESH_VERTEX = 1,
  HMESH_EDGE = 2,
  HMESH_FACE = 3,
  HMESH_VOLUME = 5,
  HMESH_FLAG = 7,
  /* Info on edge, face, */
  HMESH_HEDGE_ORDER_IN_FACE = (1|2) << HMESH_FACE,
  HMESH_HEDGE_ORDER_IN_EDGE = 1 << HMESH_EDGE,
  HMESH_FLAG_TEMP  = 1 << HMESH_FLAG,
  HMESH_VERTEX_VALENCE = 1|2|4|8
};

#define HMESH_ENVIRONMENT(__fr)\
	FRONT_ENVIRONMENT(__fr)\
	Hedge * halfedges  = (Hedge *)\
		((_front._fr)->stacks[_halfedge_]->obj);\
	NOT_UNUSED (halfedges);

#if dimension == 3

// return the "flip" hedge of h
static inline
Hedge * hedge_flip(Hedge * h) {
  return (h->flip);
}
// return the "next" hedge of h
static inline
Hedge * hedge_next(Hedge * h) {
  return (h->next);
}
// return the order of hedge in the face. 
// hface() \in {0,1,2,3}.
static inline 
Hedge * hedge_face_order(h) {
  ((h->flags&24)>>3)
}
// set the face order of hedge. i \in {0,1,2}.(4 is exceptional)
@define hface_set(h,i) {h->flags &= ~24; h->flags |= (i<<3);}
// check if h is the first or the second hedge of the full edge.
@define hedge(h) ((h->flags&4)==4)
// set the hedge. i \in {0,1}
@define hedge_set(h,i) {h->flags &=  ~4; h->flags |= (i<<2);}
//Returns the valence of "pivot" vertex of a hedge.
@define hpivot(h) ((h->v->flags) & 15)
//Reset the valence of "pivot" vertex of a hedge to 0.
@define hpivot_reset(h) {h->v->flags &= ~15;}
//Increment the valence of "pivot" of the halfedge
@define hpivot_plus(h) {++(h->v->flags);}
//Decrement the valence of "pivot" of the halfedge
@define hpivot_minus(h) {--(h->v->flags);}
//Temporary flag for temporary use only. One bit
@define hflag(h) (h->flags&32)
@define hflag_on(h) {h->v->flags |= 32;}
@define hflag_off(h) {h->v->flags &= ~32;}
//regrid status of h. 
//0: "No regrid in progress", 
//1: "split operation in progress", 
//2: "collapse operation in progress"
//3: "Don't collapse". as a collapse is in progress nearby
@define hregrid(h) (h->flags&3)
//set the regrid status. i \in {0,1,2,3} 
@define hregrid_set(h,i)  {h->flags &=  ~3; h->flags |= i;}
//next hedge (during edge splitting. To get vertices of old face)
@define hnext(h) ( hregrid(h)==1 ? h->next->next : h->next)
//vertices of hedges
@define hedge_vertices(h) {h->v, hnext(h)->v}
//vertices of faces
@define hface_vertices(h) {h->v, hnext(h)->v, hnext(hnext(h))->v}
//compare the ID of simplices in case of determining "precedence"
#if !_MPI
@define hcompare(a,b) (a->id - b->id)
#else
@define hcompare(a,b) (a->pid < b->pid == 0 ? a->alias - b->alias : a->pid - b->pid)
#endif
@define hprecedence(a,b) (hcompare(a,b) < 0 ? a : b)

/*	iterates through the hedges that share the same ..
..	"pivot" vertex of hedge __h */
@def hedge_valence_start(__h) 
{
	int __limit = 15;
	Hedge * __first = __h;
	while (__limit--){
@
@def hedge_valence_end()
		__h = hedge_flip (__h);
		__h = hedge_next (__h);
		//"next" of "flip" also has same pivot.
		if (!hcompare(__first,__h))
			break;
	}
	__h = __first; //Making sure
	if(!__limit){
		fprintf(stderr, "\nError: halfedge: Bad vertex valence. %d",
		  15-__limit);
		fflush(stderr);
		assert(false);
	}
}
@

/*	Iterate through all the hedges of the mesh*/
@def foreach_hedge(fr) 
	{
		HMESH_ENVIRONMENT(fr);
		foreach_object(_front._fr->stacks[_frontedge_]) { 
			Hedge * __h = (Hedge *) __obj;
		
@
@def end_foreach_hedge()
		end_foreach_object()
		}
	}
@

/* Iterate through all the hedges that share the same face
..	with _h. (Cyclic ordering) */
@def hface_start(__h) 
{
	//If all the hedges of hface splits, a max of 6 is expected
	int __limit = 6;
	Hedge * __first = __h;
	while (__limit--){
@
@def hface_end()
		//"next" halfedge of the same face
		__h = hedge_next(__h);
		if (!hcompare(__first,__h))
			break;
	}
	__h = __first; //Making sure
	if(__limit<3){
		fprintf(stderr, "\nWarning: Bad polygon size. %d", 6-__limit);
		fflush(stderr);
	}
}
@

/* Iterate through all the faces (or all the hedges with
.. hface() == 0 )*/
@def foreach_hface(fr) 
	foreach_hedge(fr) 
		if(hface(__h)) continue;
@
@def end_foreach_hface()
	end_foreach_hedge()
@

/* Iterate through all the full edges (or all hedges ..
..	with hedge() == 0)*/
@def foreach_fulledge(fr) 
	foreach_hedge(fr) 
		if(hedge(__h)) continue;
@
@def end_foreach_fulledge()
	end_foreach_hedge()
@

int
hmesh_valid(Front * fr){
	/* Check if a hmesh is valid*/
	HMESH_ENVIRONMENT(fr);
	assert(_front._fr->stacks[_frontedge_]);

	//checking if pointers are set 
	foreach_hedge(fr){
		assert(__h->flip);
		assert(__h->next);
		assert(__h->v);
	}

	int error = 0;

	foreach_hedge(fr){
		Frontpoint * v = __h->v;
		int valence = 0;
		//Check hedges sharing same "pivot" have the same "pivot"
		hedge_valence_start(__h){
			if(hcompare(v,__h->v)) {
				fprintf(stderr,
	"\n Error: Pivot is wrong");
				error |= 16;
			}
			valence++;
		}hedge_valence_end()
		//Check valence in [3,10)
		if(valence <3 || valence >=10){
			fprintf(stderr,
	"\n Error: Valence (of v%ld) %d not in [3,10)", 
	__h->v->alias, valence);
			error |= 1;
		}
		//Check if the valence of the pivot is set correctly
		if(hpivot(__h) != valence){
			fprintf(stderr,
	"\n Error: Valence of the pivot  .. \
	 \n .. (h%ld|v%ld) is set incorrectly as %d instead of %d", 
	__h->alias, __h->v->alias, hpivot(__h), valence);
			error |= 2;
		}
		//Check faces are triangles.
		int nv = 0, __hface = hface(__h);
		hface_start(__h) {
			nv++;
			if(__hface != hface(__h)){
				fprintf(stderr,
	"\n Error: Incorrect Face Ordering of hedges");
				error |= 4;
			}
			__hface = (__hface+1)%3;
		}hface_end()
		if(nv != 3){
			fprintf(stderr,
	"\n Error: Face is not a triangle");
			error |= 8;
		}
		//Check if "flip"s are twinned to each other.
		if(hcompare(__h,__h->flip->flip)){
			fprintf(stderr,
	"\n Error: Flips are not interconnected");
			error |= 32;
		}
		//Check if any hedge has zero length
		if(hcompare(__h->v,__h->next->v) == 0){
			fprintf(stderr,
	"\n Error: Hedge h:=(%ld, %ld) is of zero length",
	__h->v->alias, __h->next->v->alias);
			error |= 64;
		}
		//Check if any incomplete regrid operation
		if(hregrid(__h)){
			fprintf(stderr,
	"\n Error: Incomplete regrid operation.");
			error |= 128;
		}
	}

	fflush(stderr);
	return (error);
}

/**Macro to add or delete obj*/
#define add_hedge(ih,v) { \
	add_obj ( _front._fr->stacks[_frontedge_], &ih);\
	_front._fr->flags |= 16; \
	Hedge * __h = halfedges + ih;\
	__h -> alias = __h -> id = ih;\
	__h->pid = pid(); __h->flags = 0; __h -> v = v;\
	hpivot_plus(__h);\
}
//fixme: Error in fmpi
#define delete_hedge(__h) {\
	hpivot_minus(__h);\
	delete_obj ( _front._fr->stacks[_frontedge_], __h->alias);\
}

#include "hmesh-surface.h"

void 
hmesh_init(Front * fr){
	/* Create a Half-Edge Mesh data from Front data (V,E,N) */

	HMESH_ENVIRONMENT(fr);

	foreach_frontpoint(fr){
		//set the valence to zero
		frontpoint->flags &= ~15;
	}

	foreach_frontelement(fr) {
		double * t = frontelement->T;
		//creating edges
		for(int d=0; d<3; ++d) {
			Frontpoint * v = Vertexi (d, frontelement);
			long h;
			add_hedge(h,v);
			t[d] = (double) h;
		}
	}

	foreach_frontelement(fr) {
		double * t = frontelement->T;
		long ih[3], ih_[3];
		for(int d=0; d<3; ++d){
			ih[d] = (long) t[d];
			int flag = 0;
			Frontelement * n = Nbri(d,frontelement);
			for (int _d=0; _d<3; ++_d)
				if ( elid == NbrIi(_d, n) ){
	flag = 1;
	ih_[d] = (long) n->T[_d];
	break;
				}
			assert(flag);
		}
		//connecting half edges of a face using "next"/"prev"
		for(int d=0; d<3; ++d){
			Hedge * h = halfedges + ih[d],
			  * _h = halfedges + ih[(d+1)%3];
			h->next = _h;
			_h->prev = h;
			//setting the index of hedge in the face ordering. 
		  hface_set(h,d);
		}
		//connecting "flip" 
		for(int d=0; d<3; ++d){
			Hedge * h = halfedges + ih[d],
			  * _h = halfedges + ih_[d];
			h->flip = _h;
			_h->flip = h;
			//setting the index of hedge in the edge ordering.
			if(hcompare(h,_h)<0){
				hedge_set(h,0);
				hedge_set(_h,1);
			}
		}
	}

	assert(!hmesh_valid(fr));
#if _MPI
 fixme
#endif
}
//including all remesh routines of hmesh
#include "hmesh-remesh.h"

//including all additional routines related to hmesh
#include "hmesh-utils.h"
#endif
