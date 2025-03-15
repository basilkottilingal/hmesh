/* half Edge mesh - Regrid Routines */

void
hedge_split(Hedge * h){	

	HMESH_ENVIRONMENT(NULL);

	//Split edges by inserting a point in between.
	//Faces will be split later using hface_split().
	Frontpoint * v = NULL;
	if(hregrid(h->flip)){
		v = h->flip->next->v;
	}
	else {
		long iv;
		add_point(iv);
		v = frontpoints + iv;
		//v->flags &= ~15; //zero valence: redundant
		//fixme : Improve. Use Tangent plane to interpolate.
		//coord c = halfedge_ (h);
		coord c = hedge_interpolate (h, 0.5);
		memcpy (v->x, &c, 3*sizeof(double)); 
	}
	long ih;
	add_hedge(ih,v);
	Hedge * _h = halfedges + ih;
	_h->next = h->next;
	_h->flip = h->flip; 
	_h->flags = h->flags;
	hregrid_set(_h,0); //new hedge doesn't need regrid
	h->next = _h;
	if (hregrid(h->flip)) {
		h->flip->flip = _h;
		h->flip = h->flip->next;
	}
	hregrid_set(h,1); //incomplete edge operation.
}

void
hface_split(Hedge * h){

	HMESH_ENVIRONMENT(NULL);

	assert(hregrid(h));

	// There is pending hedge operation. ..
	// .. Faces are yet to be split
	Hedge * H[3] = {h, hnext(h), hnext(hnext(h))};

	int nregrid = 1;
	if (hregrid(H[1])) {
		nregrid++;
		if (hregrid(H[2]))  {
			// Special case. All 3 edges of a face are ..
			// .. simultaneously undergoing splitting. ..
			// The face is divided into 4 triangles and ..
			// .. one of them shares all the 3 edge midpoints
			Hedge * _H[3], * _F[3]; 
			long ih;
			//Creating new halfedges
			for (int d=0; d<3; ++d){
	int _d = (d+1)%3;
	Frontpoint * v = H[_d]->next->v; 
	add_hedge(ih, v);
	_H[d] = halfedges + ih;
	v = H[d]->next->v;
	add_hedge(ih, v);
	_F[d] = halfedges + ih;
			}
			//Setting new halfedges	
			for (int d=0; d<3; ++d){
	int _d = (d+1)%3;
	_H[d]->next = H[d]->next;
	_H[d]->flip = _F[d];
	hedge_set(_H[d],0);
	hface_set(_H[d], ((hface(H[_d])+1)%3) );	
	_F[d]->next = _F[_d];
	_F[d]->flip = _H[d];
	hedge_set(_F[d], 1);	
	hface_set(_F[d], d);
			}
			for (int d=0; d<3; ++d) {
	H[d]->next = _H[(d+2)%3];
	//edge splitting is complete.
	hregrid_set(H[d], 0); 
			}
			return;
		}
	}
	else if (hregrid(H[2])) {
		nregrid++;
		H[1] = H[2];
	}
	if(nregrid == 2){
		double d0 = hedge_length(H[0]),
		  d1 = hedge_length(H[1]);
		if (d0 < d1) {
			// Split in the decreasing order of the edge ..
			// .. length. The distance caclculated here is ..
			// .. the C0 length before splitting and the ..
			// .. curvature influence is not taken care.
			Hedge * temp = H[0];
			H[0] = H[1];
			H[1] = temp;
		}
	}
	for (int k=0; k<nregrid; ++k){
		// In case nregrid < 3.
		// Split all the hedges that share the face with "h".
		// .. with hregrid() == 1 (i.e hedge length > amax)
		// .. in the decreasing order of edge length.
		Hedge * A[3] = {H[k],  hnext(H[k]), hnext(hnext(H[k])) };
		assert(hregrid(A[0]) == 1);

		if(hregrid(A[1])) A[1] = A[1]->next;
		long ih;
		Frontpoint * v = A[0]->next->v;
		add_hedge(ih, v);
		Hedge * _p = halfedges + ih;
		
		v = A[2]->v;
		add_hedge(ih, v);
		Hedge * _q = halfedges + ih;
		
		_p->next = A[2];
		_p->flip = _q;
		hedge_set(_p,0);
		hface_set(_p, ((hface(A[0])+1)%3));

		_q->next = A[0]->next;
		_q->flip = _p;
		hedge_set(_q,1);
		hface_set(_q, ((hface(A[1])+1)%3));
		
		A[0]->next = _p;
		A[1]->next = _q;

		//edge splitting is complete.
		hregrid_set(A[0], 0); 
	}
}

int
hfaces_share(Hedge * h){
	//Do faces h->next->flip and 
	//h->next->next->flip share a common face?
	Hedge * p = h->next->flip, 
	  * q = h->next->next->flip;

	Hedge * H[4] = {p->next->flip, p->next->next->flip,
		q->next->flip, q->next->next->flip};
	for (int k=0; k<4; ++k){
		Hedge * _h = H[k];
		for(int _k=0; _k<3; ++_k) {
			if(k < 2) {
				hflag_off(_h);
			}
			else {
				hflag_on(_h);
			}
			_h = _h->next;
		}
	} 
	for (int k=0; k<2; ++k){
		Hedge * _h = H[k];
		for(int _k=0; _k<3; ++_k) {
			if(hflag(_h)) return 1;
			_h = _h->next;
		}
	}

	return 0;
}

int
hedge_collapse(Hedge * h){	
	// Looking for any reason to ..
	// .. abandon edge collapse.
	// .. Flag "regrid = 2" incase of collapse.

	assert(!hedge(h)); 

	//Collapsing both h and h->flip;
	Hedge * H[2] = {h, h->flip};

	//abandon in case of low valence. (v<4)
	if( hpivot(H[0]) + hpivot(H[1]) - 2   < 4)
		return 0; 
	
	//abandon in case of large valence. (v>10)
	if( hpivot(H[0]) + hpivot(H[1]) - 2   > 10)
		return 0; 

	for (int ih=0; ih<2; ++ih){
		Hedge * __h = H[ih];

		//Special care 
		//if(hfaces_share(__h))
			//return 0;	

		//abandon in case of low valence
		if( hpivot(__h->next->next) -1 < 4)
			return 0; 

		// Looking if there is any edge collapse ..
		// .. going on an edge that share a vertex ..
		// .. with this edge
		hedge_valence_start(__h)
			if(hregrid(__h) || hregrid(__h->flip))
				return 0; 
		hedge_valence_end()	

	}
	
	// Listing h and h->flip for the collapsing ..
	// operation and flag the neighborhood.
	for (int ih=0; ih<2; ++ih){
		Hedge * __h = H[ih];

		//flagging neighborhood
		hedge_valence_start(__h)
			//flag: "do not touch";
			hregrid_set(__h, 3); 
		hedge_valence_end()

		//flag: "collapsing in progress";
		hregrid_set(__h,2); 
	}

	return 1;
}

void
hface_collapse(Hedge * h){	

	assert(hregrid(h) == 2);

	HMESH_ENVIRONMENT(NULL);

	// Finish the collapse operation.
	// .. Involve deleting vertex, collapses edge/faces. 
	// .. and reconnect
	
	Hedge * H[2] = {h, h->flip};
	Frontpoint * V[2] = {H[0]->v, H[1]->v};

	//which vertex to retain. 
	int iv = hcompare(V[0], V[1]) < 0 ? 0 : 1;

	//evaluate the valence of the retained vertex
	unsigned int val =  hpivot(H[0]) + hpivot(H[1]);

	//evaluating the new coord of the retained vertex
	coord c = hedge_interpolate (h, 0.5);
	memcpy (V[iv]->x, &c, 3*sizeof(double)); 

	//reset the pivot to V[iv] for the hedges whose pivot .. 
	//.. was the deleted pivot V[!iv]
	{
		Hedge * __h = H[!iv];
		hedge_valence_start(__h)
			__h->v = V[iv];
		hedge_valence_end()
	}

	//Deleting faces by deleting 3 haledges.
	for (int ih = 0; ih<2; ++ih){
		Hedge * __h = H[ih];
		Hedge * __n = __h->next->flip, 
		  * __nn = __h->next->next->flip;
		__n->flip  = __nn;
		__nn->flip =  __n;
		//hedge_set(__n, ih);
		//hedge_set(__nn, (!ih));
		//deleting 3 hedges and thus the face 
		//delete_hedge (__h->next->next);
		//delete_hedge (__h->next);
		//delete_hedge (__h);
	}

	//reset the valence of the retained vertex.
	hpivot_reset(H[iv]);
	V[iv]->flags |= val;

	//delete the non retained vertex V[!iv];
	delete_point(V[!iv]->alias); //fixme: fmpi
	
}

void
hedge_swap(Hedge * h){	

	HMESH_ENVIRONMENT(NULL);

	if (hpivot(h) <= 6 || hpivot(h->flip) <= 6) return;
	if (hpivot(h->next->next) > 6 || 
	    hpivot(h->flip->next->next) > 6) return;

	assert(!hedge(h)); 
}

int 
hedge_split_criteria(Hedge * h){
	/* Looking for any reason to split */
	double * x[3] = {h->v->x, 
	  h->next->v->x, h->next->next->v->x};

	double l[3] = { distance(x[0], x[1]),
	  distance(x[1], x[2]), distance(x[2], x[0]) };

	if ( l[0] >  _front._regrid->amax ) 
		return 1;

	if(l[0]<l[1] || l[0]<l[2])
		return 0;

	if( htriangle_quality(x) < 0.8  
	   && l[1] > _front._regrid->amin/2
	   && l[2] > _front._regrid->amin/2 )
		return 1; 

	return 0;
}

int
hmesh_split(Front * fr){

	int nsplit = 0;

	// Set vertex normal. Required for new vertices.
	hmesh_normal(fr);

	// Split if (full) edges are to be divided/not
	foreach_fulledge(fr)
		if(hedge_split_criteria(__h) ||
 	   hedge_split_criteria(__h->flip)) {
			//Divide "twin" hedges
			hedge_split(__h->flip);
			hedge_split(__h);
			nsplit++;
		}

	//complete split operation by splitting faces	
	foreach_hedge(fr)
		if(hregrid(__h))
			hface_split(__h);

	assert(hmesh_valid(fr) < 8);
	//assert(hmesh_valid(fr) == 0);

	return nsplit;

}

int
hmesh_collapse(Front * fr){

	//collapse all hedges with length< amin.
	unsigned int status, limits = 5, 
	  abandoned = 0, collapsed = 0,
	  tcollapsed = 0;
	double amin = _front._regrid->amin;

	do 
	{
		abandoned = collapsed = 0;
		//flag for "collapse"/"abandon"
		foreach_fulledge(fr) {
			if(amin > hedge_length(__h)) {
				status = hedge_collapse(__h);
				collapsed +=  status;
				abandoned += !status;
			}
		}
			
		tcollapsed += collapsed;

		fprintf(stdout, "\n  amin%g %d %d %d", amin,
			collapsed, abandoned, limits);

		//We will abandon only in the case when ..
		//.. there is some collapsing happening around
		//if(abandoned) 
			//assert(collapsed); 
		
		if(!collapsed)
			break;
		
		//set vertex normal. 
		//fixme: optimise by evaluating only for vertices of edge.
		hmesh_normal(fr);

		//collapse all hedges (and 2 faces + 1vertex) ..
		//.. whichever flagged collapse
		foreach_fulledge(fr) 
			if(hregrid(__h) == 2)
				hface_collapse(__h);

		foreach_hedge(fr) {
			if(hregrid(__h) == 2 || 
			   hregrid(__h->next) == 2 || 
			   hregrid(__h->next->next) == 2) 
			{
				delete_hedge (__h);
				if(hregrid(__h) == 2){
	hedge_set(__h->next->flip, 0);
	hedge_set(__h->next->next->flip, 1);
				}
			}
			if(hregrid(__h) == 3)
				hregrid_set (__h, 0);
		}
	} while ( abandoned && limits--);

	if(abandoned) {
		fprintf(stdout, 
	"\n  WARNING: There are non-collapsed small edges");
		fflush(stdout);
	}

	assert(hmesh_valid(fr) < 8);
	//assert(hmesh_valid(fr) == 0);

	return (tcollapsed);

}

void
hmesh_smooth(Front * fr){
	//Laplacian smoothing of surface mesh. 
	//.. (avoid component perpendicualr to tgt plane

	foreach_frontpoint(fr) {
		//new coordinate of vertex (will be stored in 
		//.. in frontpoint->t)
		double * _x = frontpoint->t;
		_x[0] = 0.; _x[1] = 0; _x[2] = 0.;
	}
	//sum of all vertex in the fan
	foreach_hedge(fr){
		double * _x = __h->v->t, * p = hnext(__h)->v->x;
		_x[0] += p[0]; _x[1] += p[1]; _x[2] += p[2];
	}

	//vertex normal (stored in frontpoint-T)			
	hmesh_normal(fr);

	// average by valence and move along the tgt .. 
	// plane toward the barycenter of the "fan".

	foreach_frontpoint(fr) {

		// the new coordinate of vertex
		double * _x = frontpoint->t,
		  * x = frontpoint->x, * n = frontpoint->T;

		int val = frontpoint->flags & 15;
		_x[0] = _x[0]/val; _x[1] = _x[1]/val; _x[2] = _x[2]/val;

		// projection with normal vector ..
		// .. (component out of tgt plane)
		double p = (_x[0] - x[0])*n[0] + (_x[1] - x[1])*n[1] 
		  + (_x[2] - x[2])*n[2];

		// the new coord after removing the ..
		// .. component out of plane
		x[0] = _x[0] - p*n[0];
		x[1] = _x[1] - p*n[1];
		x[2] = _x[2] - p*n[2];
	}
}

/* Remesh Algorithm */
int
hmesh_remesh(Front * fr){
	int change = 0;
	for(int k=0; k<6; ++k){
		int a = hmesh_split(fr);
		int b = hmesh_collapse(fr);
		//replace it with quality check.
		if(!(a||b))
			break;
		else { 
			hmesh_smooth(fr);
			change++;
		}
	}
	return change;
}
