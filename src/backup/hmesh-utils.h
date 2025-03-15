int 
write_hedges(Front * fr, char * file) {
	if (fr->stacks[_frontedge_] == NULL) {
		fprintf(stderr, "Half-Edge Mesh data not yet initizlized.");
		fflush(stderr);
		return 0;
	}
	/* Writes edges data to a single ASCII files (space         .. 
	.. separated). You can plot from the file in gnuplot. Refer ..
	.. https://github.com/basilkottilingal/FT/blob/main/test/basics/halfedge.c ..
	.. for example. (Go to the ##Results part of the .c file    ..
	.. mentioned above). WARNING: The file written doesn't have .. 
  .. connection data & cannot recreate the front from the file*/

	if(!file) {
		fprintf(stderr, "\nWARNING in write_hedges(): \
		  \n\tWriting in default file");
	}
	
  char def[] = "default-hedges.dat",
	    * name = file ? file : def;
  FILE * _fp = fopen(name, "w");

  if (!_fp){
		fprintf(stderr, "\nCannot open file in write_hedges()");
		fflush(stderr);
		return 0;
  }

	long n = 0, ng = 0;
	foreach_fulledge(fr) 
		n++; //number of (full)edges.

#if _MPI
	long nall [npe()];
	MPI_Allgather(&n, 1,  MPI_INT, nall, 1, 
	  MPI_INT, MPI_COMM_WORLD);
	for(int p=0; p<pid(); p++) 
		ng += nall[p];
#endif
	
	//writing coordinates
	char space[201];
	memset(space, ' ', 200);
	int jump = 2*26*dimension;
	space[jump] = '\n';
	jump++;
	space[jump] = '\0';

	for(long i=ng; i<ng+n; ++i){
		fseek (_fp, i*jump, SEEK_SET);
		fprintf(_fp, "%s", space);
	}

	long offset = ng*jump;
	foreach_fulledge(fr){ 
		fseek (_fp, offset, SEEK_SET);	
		Frontpoint * _p[2] = hedge_vertices(__h);
		for(int h=0; h<2; ++h)
			for(int d=0; d<dimension; ++d)
				fprintf(_fp, "%.15e ", _p[h]->x[d]);
		fprintf(_fp, "%d %d ", hpivot(__h), hpivot(hnext(__h)));
		offset += jump;
	}

	fclose(_fp);

	return 1;	
}

int 
write_faces(Front * fr, char * file) {
	if (fr->stacks[_frontedge_] == NULL) {
		fprintf(stderr, "Half-Edge Mesh data not yet initizlized.");
		fflush(stderr);
		return 0;
	}
	/* Writes face data to a single ASCII files (space         .. 
	.. separated). You can plot from the file in gnuplot. Refer ..
	.. https://github.com/basilkottilingal/FT/blob/main/test/basics/halfedge.c ..
	.. for example. (Go to the ##Results part of the .c file    ..
	.. mentioned above). WARNING: The file written doesn't have .. 
  .. connection data & cannot recreate the front from the file*/

	if(!file) {
		fprintf(stderr, "\nWARNING in write_hfaces(): \
		  \n\tWriting in default file");
	}
	
  char def[] = "default-hedges.dat",
	    * name = file ? file : def;
  FILE * _fp = fopen(name, "w");

  if (!_fp){
		fprintf(stderr, "\nCannot open file in write_hfaces()");
		fflush(stderr);
		return 0;
  }

	long n = 0, ng = 0;
	foreach_hface(fr) 
		n++; //number of faces.

#if _MPI
	long nall [npe()];
	MPI_Allgather(&n, 1,  MPI_INT, nall, 1, 
	  MPI_INT, MPI_COMM_WORLD);
	for(int p=0; p<pid(); p++) 
		ng += nall[p];
#endif
	
	//writing coordinates
	char space[201];
	memset(space, ' ', 200);
	int jump = 3*25*dimension;
	space[jump] = '\n';
	jump++;
	space[jump] = '\0';

	for(long i=ng; i<ng+n; ++i){
		fseek (_fp, i*jump, SEEK_SET);
		fprintf(_fp, "%s", space);
	}

	long offset = ng*jump;
	foreach_hface(fr){ 
		fseek (_fp, offset, SEEK_SET);	
		Frontpoint * _p[3] = hface_vertices(__h);
		for(int h=0; h<3; ++h)
			for(int d=0; d<dimension; ++d)
				fprintf(_fp, "%.15e ", _p[h]->x[d]);
		offset += jump;
	}

	fclose(_fp);

	return 1;	
}

int 
write_hneighborhood(Front * fr, long * V, int nv, char * file) {
	if (fr->stacks[_frontedge_] == NULL) {
		fprintf(stderr, "Half-Edge Mesh data not yet initizlized.");
		fflush(stderr);
		return 0;
	}
	// Printing the neighborhood of vertices V[]. ..
	// .. Assume nv<5. Otherwise it is extremely costly
	assert(nv<5);

	if(!file) {
		fprintf(stderr, "\nWARNING: No file name mentioned ");
		return 0;
	}

	int flag = 0;
	foreach_hedge(fr)
		for(int iv = 0; iv<nv; ++iv)
			if(__h->v->alias == V[iv])
				flag = 1;

	if(!flag)
		return 0;

  char def[] = "neighborhood",
	    * name = file ? file : def;
  FILE * _fp = lfopen(name, "w");

  if (!_fp){
		fprintf(stderr, "\nCannot open file in write_hneighborhood()");
		fflush(stderr);
		return 0;
  }

	foreach_hedge(fr){
		for(int iv = 0; iv<nv; ++iv) {
			if(__h->v->alias == V[iv]){
				hedge_valence_start(__h)

					Hedge * H[2] = {__h, __h->next};
					for(int k=0; k<2;++k){
	fprintf(_fp, "\n");
	Frontpoint * _p[2] = hedge_vertices(H[k]);
	for(int h=0; h<2; ++h)
		for(int d=0; d<dimension; ++d)
			fprintf(_fp, "%.15e ", _p[h]->x[d]);
	fprintf(_fp, "%d %d ", hpivot(H[k]), hpivot(hnext(H[k])));
	fprintf(_fp, "%ld %ld ", H[k]->v->alias, hnext(H[k])->v->alias);
					}

				hedge_valence_end()
			}
		}
	}

	fclose(_fp);

	return 1;	
}
