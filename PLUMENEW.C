/*
 *
 * plume.c
 * Saturday, 10/30/1993.
 *
 */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <GnuArg.h>
#include <GnuMath.h>
#include <GnuFile.h>
#include <math.h>


BIG   X   , Y , DX, DY, T ;
BIG   Size, C0, A , DL, DT;
BIG   X00  , Y00, VX, DELX, DELY; 

BIG pi=3.1415926535;

char sz [256];

BIG z [50][50];


void Usage (void)
   {
   printf ("   PLUME   plume array generator\n");
   printf ("\n");
   printf ("   USAGE: PLUME [options]\n");
   printf ("\n");
   printf ("   WHERE: options are 0 or more of:\n");
   printf ("\n");
   printf ("      Graph=#    .....  use this to graph output.\n");
   printf ("      Precision=# ....  number of decimals to print\n");
   printf ("\n");
   printf ("      X=# ............  left side of array grid\n");
   printf ("      Y=# ............  top of array grid\n");
   printf ("      DELX=# .........  X Resolution\n");
   printf ("      DELY=# .........  X Resolution\n");
   printf ("      T=# ............  Time\n");
   printf ("      Size=# .........  Size of output grid\n");
   printf ("      C0=# ...........  Injected concentration\n");
   printf ("      A=# ............  Injection Area\n");
   printf ("      DL=# ...........  Longitudional Dispersion\n");
   printf ("      DT=# ...........  Transversal Dispersion\n");
   printf ("      X00=# ..........  Injection point\n");
   printf ("      Y00=# ..........  Injection point\n");
   printf ("      VX=# ...........  Groundwater Velocity\n");
   printf ("\n");
   printf ("   values may also be stored in plume.cfg.\n");
   exit (0);
   }



USHORT LoadCfg (void)
   {
   FILE *fp;
   BIG  bgTmp;

   if (!(fp = fopen ("plume.cfg", "rt")))
      return printf ("Unable to open cfg file PLUME.CFG\n");

   while (FilReadLine (fp, sz, ";", sizeof sz) != -1)
      {
      AToBIG (&bgTmp, sz);
      }
   fclose (fp);
   }


void DumpVars (void)
   {
   printf ("X     = %Lf\n", X   );
   printf ("Y     = %Lf\n", Y   );
   printf ("DELX  = %Lf\n", DELX);
   printf ("DELY  = %Lf\n", DELY);
   printf ("T     = %Lf\n", T   );
   printf ("Size  = %Lf\n", Size);
   printf ("C0    = %Lf\n", C0  );
   printf ("A     = %Lf\n", A   );
   printf ("DL    = %Lf\n", DL  );
   printf ("DT    = %Lf\n", DT  );
   printf ("X00   = %Lf\n", X00  );
   printf ("Y00   = %Lf\n", Y00  );
   printf ("VX    = %Lf\n", VX  );
   }




BIG CalcVal (BIG iy, BIG ix)
   {
   BIG a, b, c, d, e, f, g;

   a = ((iy - Y00) * (iy - Y00)) / (4 * DT * T);

   b = (ix - (X00 + VX * T));
   c = (b * b) / (4 * DL * T);

   d = powl (DL * DT, 0.5);
   e = 4 * pi * T * d;
   f = (C0 * A) / e;

   g = f * expl (- c - a);

   return g;
   }



int main (int argc, char *argv[])
   {
   BIG ix, iy, GraphVal, Precision, cv, xcv, xxcv;
   USHORT uPrecision, i, j;
   BOOL   bGraph;

   xcv = x * cv;
   xxcv = (x ^ 2) * cv;

   ArgBuildBlk ("^Graph# ^Precision# " 
                "^X# ^Y# ^DX# ^DY# ^T# ^Size# ^Debug "
                "^C0# ^A# ^DL# ^DT# ^X00# ^Y00# ^VX# ^DELX# ^DELY# "
                );

   LoadCfg ();

   if (argc <2)
      Usage ();

   AToBIG (&X,   "@X   ");
   AToBIG (&Y,   "@Y   ");
   AToBIG (&DELX,"@DELX");
   AToBIG (&DELY,"@DELY");
   AToBIG (&T,   "@T   ");
   AToBIG (&Size,"@Size");
   AToBIG (&C0,  "@C0  ");
   AToBIG (&A,   "@A   ");
   AToBIG (&DL,  "@DL  ");
   AToBIG (&DT,  "@DT  ");
   AToBIG (&X00, "@X00  ");
   AToBIG (&Y00, "@Y00  ");
   AToBIG (&VX,  "@VX  ");
      
   if (ArgFillBlk (argv))
      {
      fprintf (stderr, "%s\n", ArgGetErr ());
      Usage ();
      }

   if (ArgIs ("X"))    AToBIG (&X    , ArgGet ("X", 0))   ;
   if (ArgIs ("Y"))    AToBIG (&Y    , ArgGet ("Y", 0))   ;
   if (ArgIs ("DELX")) AToBIG (&DELX , ArgGet ("DX", 0))  ;
   if (ArgIs ("DELY")) AToBIG (&DELY , ArgGet ("DY", 0))  ;
   if (ArgIs ("T"))    AToBIG (&T    , ArgGet ("T", 0))   ;
   if (ArgIs ("Size")) AToBIG (&Size , ArgGet ("Size", 0));
   if (ArgIs ("C0"))   AToBIG (&C0   , ArgGet ("C0", 0))  ;
   if (ArgIs ("A"))    AToBIG (&A    , ArgGet ("A", 0))   ;
   if (ArgIs ("DL"))   AToBIG (&DL   , ArgGet ("DL", 0))  ;
   if (ArgIs ("DT"))   AToBIG (&DT   , ArgGet ("DT", 0))  ;
   if (ArgIs ("X00"))  AToBIG (&X00   , ArgGet ("X00", 0))  ;
   if (ArgIs ("Y00"))  AToBIG (&Y00   , ArgGet ("Y00", 0))  ;
   if (ArgIs ("VX"))   AToBIG (&VX   , ArgGet ("VX", 0))  ;

   if (ArgIs ("Precision"))   
      AToBIG (&Precision, ArgGet ("Precision", 0));
   else
      Precision = 5;
   uPrecision = (USHORT) Precision;


   bGraph = FALSE;
   if (ArgIs ("Graph"))
      {
      bGraph = TRUE;
      AToBIG (&GraphVal, ArgGet ("Graph", 0));
      }

   if (ArgIs ("Debug"))
      DumpVars ();

   printf ("%u\n", (USHORT) Size);
   for (iy = Y, i=0; i<Size; i++, iy+=DELY)
      {
      for (ix = X, j=0; j<Size; j++, ix+=DELX)
         {
         cv = CalcVal (iy, ix);

         z[i][j] = cv;

         if (bGraph)
            {
            printf ("%c", (cv > GraphVal ? '*' : '.'));
            }
         else
            {
            printf ("%.*Lf ", uPrecision, cv);
            }
         }
      printf ("\n");
      }   
   printf ("\n\n");

   /*--- print averages ---*/
   for (i = 1; i < (USHORT) Size-1; i+=2)
      {
      for (j = 1; j < (USHORT) Size-1; j+=2)
         {
         cv =(z[i + -1][j + -1] *  1+
              z[i +  0][j + -1] *  4+
              z[i +  1][j + -1] *  1+
              z[i + -1][j +  0] *  4+
              z[i +  0][j +  0] *  16+
              z[i +  1][j +  0] *  4+
              z[i + -1][j +  1] *  1+
              z[i +  0][j +  1] *  4+
              z[i +  1][j +  1] *  1);
         cv /= 36;

         if (bGraph)
            {
            printf ("%c", (cv > GraphVal ? '*' : '.'));
            }
         else
            {
            printf ("%.*Lf ", uPrecision, cv);
            printf ("%.*Lf ", uPrecision, xcv);
            printf ("%.*Lf ", uPrecision, xxcv);
            }
         }
      printf ("\n");
      }

   return 0;
   }

