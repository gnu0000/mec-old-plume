/*
 *
 * plume.c
 * Saturday, 10/30/1993.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GnuType.h>
#include <GnuArg.h>
#include <GnuMath.h>
#include <GnuFile.h>
#include <GnuCfg.h>
#include <GnuStr.h>


char sz [256];

#define SIZ 40

BIG z  [SIZ][SIZ];   // plume array
BIG z1 [SIZ][SIZ];   // result array 1
BIG z2 [SIZ][SIZ];   // result array 1

PSZ    pszPlumeDat   [SIZ];

USHORT uOutputType   [SIZ];
PSZ    pszOutputExpr [SIZ];

USHORT uSteps, uPrecision;


void Usage (void)
   {
   printf ("   PLUME   plume array generator\n");
   printf ("\n");
   printf ("   USAGE: PLUME [options]\n");
   printf ("\n");
   printf ("   WHERE: options are 0 or more of:\n");
   printf ("\n");
   printf ("      /Precision=# ....  number of decimals to print\n");
   printf ("      variable=value ..  set a variable to a value (no spaces)\n");
   printf ("\n");
   printf ("   values may also be stored in plume.cfg.\n");
   printf ("\n");
   printf ("EXAMPLES:\n");
   printf ("\n");
   printf ("   PLUME @T=1770\n");
   printf ("   PLUME @Steps=10 @T=1770 /precision=3\n");
   printf ("\n");
   exit (0);
   }

PSZ Error (PSZ psz1, PSZ psz2)
   {
   printf ("Error: ");
   printf (psz1, psz2);
   exit (1);
   return NULL;
   }

void MathError (PSZ pszLine)
   {
   PSZ psz;
   USHORT uTmp, i;

   MthIsError (&psz, &uTmp);
   printf ("Math Error: %s\n", psz);
   if (pszLine)
      {
      printf ("%s\n", pszLine);
      for (i=0; i<uTmp; i++)
         printf (" ");
      printf ("^\n");
      }
   exit (1);
   }


void SetVar (PSZ pszVar, BIG bgVal)
   {
   sprintf (sz, "%s = %Lf", pszVar, bgVal);
   bgVal = AToBIG (sz);
   }



void LoadCfg (void)
   {
   FILE *fp;
   BIG  bgTmp;
   PSZ  psz;
   USHORT i, uIdx;

   for (i=0; i<SIZ; i++)
      {
      pszPlumeDat[i] = NULL;
      uOutputType[i] = 0;
      }
    

   /*
    * read default section
    */
   if (!(fp = CfgFindSection ("plume.cfg", "defaults")))
      {
      CfgGetError (&psz);
      Error (psz, "");
      }
   while (FilReadLine (fp, sz, ";", sizeof sz) != -1)
      {
      if (CfgEndOfSection (sz))
         break;

      if (StrBlankLine (sz))
         continue;

      bgTmp = AToBIG (sz);
      if (!MthValid (bgTmp))
         MathError (sz);
      }
   fclose (fp);

   /*
    * read plume section
    */
   uIdx = 0;
   if (!(fp = CfgFindSection ("plume.cfg", "plume")))
      {
      CfgGetError (&psz);
      Error (psz, "");
      }

   while (FilReadLine (fp, sz, ";", sizeof sz) != -1)
      {
      if (CfgEndOfSection (sz))
         break;

      if (StrBlankLine (sz))
         continue;

      pszPlumeDat[uIdx++] = strdup (sz);
      }
   fclose (fp);

   /*
    * read output section
    */
   if (!(fp = CfgFindSection ("plume.cfg", "output")))
      {
      CfgGetError (&psz);
      Error (psz, "");
      }

   uIdx = 0;
   while (FilReadLine (fp, sz, ";", sizeof sz) != -1)
      {
      if (CfgEndOfSection (sz))
         break;

      StrClip (StrStrip (sz, " \t"), " \t");

      if (! *sz)
         continue;

      if (!strnicmp (sz, "plumegraph", 10))
         {
         uOutputType [uIdx] = 4;
         pszOutputExpr[uIdx] = strdup (StrStrip (sz+10, " \t"));
         }
      else if (!strnicmp (sz, "plume", 5))
         {
         uOutputType [uIdx] = 1;
         pszOutputExpr[uIdx] = strdup (StrStrip (sz+5, " \t"));
         }
      else if (!strnicmp (sz, "process2", 8))
         {
         uOutputType [uIdx] = 3;
         pszOutputExpr[uIdx] = strdup (StrStrip (sz+8, " \t"));
         }
      else if (!strnicmp (sz, "process", 7))
         {
         uOutputType [uIdx] = 2;
         pszOutputExpr[uIdx] = strdup (StrStrip (sz+7, " \t"));
         }
      else
         Error ("Unknown output type: %s", sz);
      uIdx++;
      }
   fclose (fp);
   }




BIG CalcPlumeVal (BIG bgY, BIG bgX)
   {
   BIG bgTmp;
   USHORT i;

   SetVar ("@Y", bgY);
   SetVar ("@X", bgX);

   for (i=0; pszPlumeDat[i]; i++)
      {
      bgTmp = AToBIG (pszPlumeDat[i]);
      if (!MthValid (bgTmp))
         MathError (pszPlumeDat[i]);
      }
   bgTmp = AToBIG ("@point");
   return bgTmp;
   }



BIG CalcProcessVal (USHORT y, USHORT x, )
   {
   BIG bgVal;

   bgVal =(z[y + -1][x + -1] * 1+
        z[y +  0][x + -1] * 4+
        z[y +  1][x + -1] * 1+
        z[y + -1][x +  0] * 4+
        z[y +  0][x +  0] * 16+
        z[y +  1][x +  0] * 4+
        z[y + -1][x +  1] * 1+
        z[y +  0][x +  1] * 4+
        z[y +  1][x +  1] * 1);
   return bgVal / 36.0;
   }



void PrintPlume (PSZ psz)
   {
   USHORT y, x;
   BIG   bgVar;

   printf ("[Printing Plume (%s) %d x %d ]\n", psz, uSteps, uSteps);
   for (y=0;  y<uSteps; y++)
      {
      for (x=0; x<uSteps; x++)
         {
         SetVar ("@point", z[y][x]);
         bgVar = AToBIG (psz);
         if (!MthValid (bgVar))
            MathError (psz);
         printf ("%.*Lf ", uPrecision, bgVar);
         }
      printf ("\n");
      }
   printf ("\n");
   }


void PrintGraph (PSZ psz)
   {
   USHORT y, x;
   BIG   bgVar;

   bgVar = AToBIG (psz);

   printf ("[Printing Plume graph ( > %s) %d x %d ]\n", psz, uSteps, uSteps);
   for (y=0; y<uSteps; y++)
      {
      for (x=0; x<uSteps; x++)
         {
         if (z[y][x] > bgVar)
            printf ("*");
         else
            printf (".");
         }
      printf ("\n");
      }
   printf ("\n");
   }



void PrintResult1 (PSZ psz)
   {
   USHORT y, x;
   BIG   bgVar;

   printf ("[Printing Result (%s) %d x %d ]\n", psz, uSteps, uSteps);

   for (y=0; (y+1)*2 < uSteps; y++)
      {
      for (x=0; (x+1)*2 < uSteps; x++)
         {
         SetVar ("@point", z1[y][x]);
         bgVar = AToBIG (psz);
         if (!MthValid (bgVar))
            MathError (psz);
         printf ("%.*Lf ", uPrecision, bgVar);
         }
      printf ("\n");
      }
   printf ("\n");
   }


void PrintResult2 (PSZ psz)
   {
   USHORT y, x;
   BIG   bgVar;

   printf ("[Printing Result2 (%s) %d x %d ]\n", psz, uSteps, uSteps);

   for (y=0; y+2 < uSteps; y++)
      {
      for (x=0; x+2 < uSteps; x++)
         {
         SetVar ("@point", z2[y][x]);
         bgVar = AToBIG (psz);
         if (!MthValid (bgVar))
            MathError (psz);
         printf ("%.*Lf ", uPrecision, bgVar);
         }
      printf ("\n");
      }
   printf ("\n");
   }


int main (int argc, char *argv[])
   {
   BIG    bgYStart, bgYEnd, bgXStart, bgXEnd;
   BIG    bgYStep, bgXStep, bgY, bgX, bgTmp, bgSteps;
   USHORT y, x, i;

   ArgBuildBlk ("*^Precision# *^Debug ");

   if (argc <2)
      Usage ();

   LoadCfg ();

   if (ArgFillBlk (argv))
      {
      fprintf (stderr, "%s\n", ArgGetErr ());
      Usage ();
      }

   /*--- process variable on cmd line ---*/
   for (i=ArgIs (NULL); i; i--)
      {
      bgTmp = AToBIG (ArgGet (NULL, i-1));
      if (!MthValid (bgTmp))
         MathError (ArgGet (NULL, i-1));
      }

   bgXStart= AToBIG ("@XStart");
   bgYStart= AToBIG ("@YStart");
   bgXEnd  = AToBIG ("@XEnd  ");
   bgYEnd  = AToBIG ("@YEnd  ");
   bgSteps = AToBIG ("@Steps ");
   uSteps = (USHORT) bgSteps;

   /*--- read precision ---*/
   uPrecision = 5;
   if (ArgIs ("Precision"))   
      {
      bgTmp = AToBIG (ArgGet ("Precision", 0));
      uPrecision = (USHORT) bgTmp;
      }

   bgYStep = (bgYEnd - bgYStart) / bgSteps;
   bgXStep = (bgXEnd - bgXStart) / bgSteps;

   /*--- create plume array ---*/
   printf ("Calculating plume ");
   for (y=0, bgY=bgYStart; y<uSteps; y++, bgY+=bgYStep)
      {
      printf (".");
      for (x=0, bgX=bgXStart; x<uSteps; x++, bgX+=bgXStep)
         z[y][x] = CalcPlumeVal (bgY, bgX);
      }
   printf ("\n");

   /*--- create non overlapping result array ---*/
   for (y=0; (y+1)*2 < uSteps; y++)
      for (x=0; (x+1)*2 < uSteps; x++)
         z1[y][x] = CalcProcessVal (y*2+1, x*2+1);

   /*--- create overlapping result array ---*/
   for (y=0; y+2 < uSteps; y++)
      for (x=0; x+2 < uSteps; x++)
         z2[y][x] = CalcProcessVal (y+1, x+1);


   /*--- print outputs ---*/
   for (i=0; uOutputType[i]; i++)
      {
      switch (uOutputType[i])
         {
         case 1: PrintPlume   (pszOutputExpr[i]); break;
         case 2: PrintResult1 (pszOutputExpr[i]); break;
         case 3: PrintResult2 (pszOutputExpr[i]); break;
         case 4: PrintGraph   (pszOutputExpr[i]); break;
         }
      }
   printf ("Done.");
   return 0;
   }



