#include <common.h>
#include <hmesh.h>

int main() {

  void ** mem1, ** mem2, ** mem3;
  mem1 = mem2 = mem3 = NULL;

  _HmeshArray * att = HmeshArray("s", 8, &mem1);
  _Index iblock = 3;
  while(iblock--)  {
    if( !HmeshArrayAdd( att, iblock, &mem1 ) ) {
      HmeshError("main() : cannot create "
        "iblock %d in attr \"%s\"", iblock, att->name);
      continue;
    }

    double * val = (double *) mem1[iblock];
    //_Index imax = (_Index) MemblockSize();
    for(_Index i=0; i<HmeshTpoolBlockSize(); ++i, ++val)
      *val = 0.011;
  }

  iblock = 5;
  while(iblock--)  {
    /* NOTE iblock 4,3 should give error */
    if( HmeshArrayRemove( att, iblock, &mem1) ) {
      HmeshError("main() : cannot remove "
        "iblock %d in attr \"%s\"", iblock, att->name);
    }
  }

  if( HmeshArrayDestroy(att, &mem1) ) {
      HmeshError("main() : att remove err");
  }
  HmeshErrorFlush(2);

  /* warning for no name */
  _HmeshArray * att1 = HmeshArray(NULL, 8, &mem2);
  _HmeshArray * att2 = HmeshArray("", 8, &mem3);
  HmeshArrayDestroy(att1, &mem2);
  HmeshArrayDestroy(att2, &mem3);
  HmeshErrorFlush(2);


  char aname[200] = "this_is_a_very_long_attribute_name_"
    "which_can_be_potentially_trunked_without_warning_and_can_cause_problem";
  mem2 = NULL;
  att = HmeshArray(aname, 1, &mem2);
  if(strcmp(aname, att->name))
    HmeshError("atttribute name trunked to '%s'", 
      att->name);
  HmeshArrayDestroy(att, &mem2);
  HmeshErrorFlush(2);

  return 0;
}
