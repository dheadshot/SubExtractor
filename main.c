#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tstamp.h"

#define MAXBUFFER 512

#define getrange getsrtrange

int vttrangetype(char *rangestr)
{
  int rtype = 0, i, b=0, n1=0, n2=0;
  for (i=0;rangestr[i]!=0;i++)
  {
    if (rangestr[i] == ':')
    {
      if (b) n2++; else n1++;
    }
    if (rangestr[i] == '>') b++;
    if (b && n2 && rangestr[i]==' ') break;
  }
  if (n1 == 2) rtype |= 1;
  if (n2 == 2) rtype |= 2;
  if (n1<1 || n1>2 || n2<1 || n2>2) return -1;
  return rtype;
}

int getvttrange(char *rangestr, tstamp *t1, tstamp *t2)
{
  if (!rangestr || !t1 || !t2) return 0;
  t1->h = 0;
  t2->h = 0;
  switch (vttrangetype(rangestr))
  {
    case 0:
      if (sscanf(rangestr, "%u:%u.%u --> %u:%u.%u", 
                 &(t1->m), &(t1->s), &(t1->mil),
                 &(t2->m), &(t2->s), &(t2->mil)) != 6)
      {
        fprintf(stderr, "Warning: Invalid format \"%s\"! Ignoring this range.\n", rangestr);
        return -1;
      }
    break;
    
    case 1:
      if (sscanf(rangestr, "%u:%u:%u.%u --> %u:%u.%u", 
                 &(t1->h), &(t1->m), &(t1->s), &(t1->mil),
                 &(t2->m), &(t2->s), &(t2->mil)) != 7)
      {
        fprintf(stderr, "Warning: Invalid format \"%s\"! Ignoring this range.\n", rangestr);
        return -1;
      }
    break;
    
    case 2:
      if (sscanf(rangestr, "%u:%u.%u --> %u:%u:%u.%u", 
                 &(t1->m), &(t1->s), &(t1->mil),
                 &(t2->h), &(t2->m), &(t2->s), &(t2->mil)) != 7)
      {
        fprintf(stderr, "Warning: Invalid format \"%s\"! Ignoring this range.\n", rangestr);
        return -1;
      }
    break;
    
    case 3:
      if (sscanf(rangestr, "%u:%u:%u.%u --> %u:%u:%u.%u", 
                 &(t1->h), &(t1->m), &(t1->s), &(t1->mil),
                 &(t2->h), &(t2->m), &(t2->s), &(t2->mil)) != 8)
      {
        fprintf(stderr, "Warning: Invalid format \"%s\"! Ignoring this range.\n", rangestr);
        return -1;
      }
    break;
    
    default:
      fprintf(stderr, "Warning: Invalid format \"%s\"! Ignoring this range.\n", rangestr);
      return -1;
    break;
  }
  
#ifdef DEBUG
  printf("Testing range: %u:%u:%u.%u - %u:%u:%u.%u .\n", t1->h, t1->m, t1->s, t1->mil, t2->h, t2->m, t2->s, t2->mil);
#endif
  
  return 1;
}

int getsrtrange(char *rangestr, tstamp *t1, tstamp *t2)
{
  if (!rangestr || !t1 || !t2) return 0;
  if (sscanf(rangestr, "%u:%u:%u,%u --> %u:%u:%u,%u", 
             &(t1->h), &(t1->m), &(t1->s), &(t1->mil),
             &(t2->h), &(t2->m), &(t2->s), &(t2->mil)) != 8)
  {
    fprintf(stderr, "Warning: Invalid format \"%s\"! Ignoring this range.\n", rangestr);
    return -1;
  }
  return 1;
}

int inrange(tstamp *t, tstamp *rs, tstamp *re)
{
  if (t->h < rs->h) return 0;
  if (t->h > re->h) return 0;
  if (t->h == rs->h)
  {
    if (t->m < rs->m) return 0;
    if (t->m == rs->m)
    {
      if (t->s < rs->s) return 0;
      if (t->s == rs->s)
      {
        if (t->mil < rs->mil) return 0;
      }
    }
  }
  if (t->h == re->h)
  {
    if (t->m > re->m) return 0;
    if (t->m == re->m)
    {
      if (t->s > re->s) return 0;
      if (t->s == re->s)
      {
        if (t->mil > re->mil) return 0;
      }
    }
  }
  return 1;
}

int outputsrtsubs(FILE *ifp, FILE *ofp, tstamp *t)
{
  char buffer[MAXBUFFER] = "";
  
  if (!ifp || !ofp || !t) return 0;
  
  tstamp rs, re;
  memset(&rs, 0, sizeof(tstamp));
  memset(&re, 0, sizeof(tstamp));
  
  int err = 0, done = 0;
  
  do
  {
    if (!fgets(buffer, MAXBUFFER, ifp))
    {
      err = 1;
      if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
      break;
    }
#ifdef DEBUG
    printf("--Sequence %d\n", atoi(buffer));
#endif
    if (!fgets(buffer, MAXBUFFER, ifp))
    {
      err = 1;
      if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
      break;
    }
    if (getrange(buffer, &rs, &re)==1)
    {
      if (inrange(t, &rs, &re)) done = 1;
    }
    
    if (!done)
    {
      do
      {
        if (!fgets(buffer, MAXBUFFER, ifp))
        {
          err = 1;
          if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
          break;
        }
      } while (buffer[0] != '\r' && buffer[0] != '\n');
      if (err) break;
    }
    
  } while (!done);
  
  if (!err)
  {
    do
    {
      if (!fgets(buffer, MAXBUFFER, ifp))
      {
        err = 1;
        if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
        break;
      }
      if (buffer[0]>=' ') fprintf(ofp, "%s", buffer);
    } while (buffer[0] != '\r' && buffer[0] != '\n');
  }
  if (err && !feof(ifp))
  {
    return -1;
  }
  if (done) return 1;
  return 2;
}

int outputvttsubs(FILE *ifp, FILE *ofp, tstamp *t)
{
  char buffer[MAXBUFFER] = "";
  char buffer2[MAXBUFFER] = "";
  int intag = 0, i, n = 0;
  
  if (!ifp || !ofp || !t) return 0;
  
  tstamp rs, re;
  memset(&rs, 0, sizeof(tstamp));
  memset(&re, 0, sizeof(tstamp));
  
  int err = 0, done = 0;
  
  do
  {
    if (!fgets(buffer, MAXBUFFER, ifp))
    {
      err = 1;
      if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
      break;
    }
    n++;
    if (memcmp(buffer, "WEBVTT", 6) == 0)
    {
      //Read until double newline
      do
      {
        if (!fgets(buffer, MAXBUFFER, ifp))
        {
          err = 1;
          if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
          break;
        }
        n++;
#ifdef DEBUG
        printf("Metadata: \"%s\"\n", buffer);
#endif
      } while (buffer[0] != '\r' && buffer[0] != '\n');
      if (err) break;
      
      continue;
    }
    else if (buffer[0] == '\r' || buffer[0] == '\n') continue;
    else if (vttrangetype(buffer)==-1)
    {
#ifdef DEBUG
      printf("--Sequence %d, %s", atoi(buffer), buffer);
#endif
      do
      {
        if (!fgets(buffer, MAXBUFFER, ifp))
        {
          err = 1;
          if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
          break;
        }
        n++;
      } while (buffer[0]!='\r' || buffer[0]!='\n');
    }
    if (getvttrange(buffer, &rs, &re)==1)
    {
      if (inrange(t, &rs, &re)) done = 1;
    }
    
    if (!done)
    {
      /*do
      {*/
        do
        {
          if (!fgets(buffer, MAXBUFFER, ifp))
          {
            err = 1;
            if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
            break;
          }
          n++;
          //if (buffer[0] == ' ' && (buffer[1]=='\n' || buffer[1]=='\r')) break;
        } while (buffer[0] != '\r' && buffer[0] != '\n');
#ifdef DEBUG
        printf("End Section at %d\n", n);
#endif
        /*if (err) break;
        if (!fgets(buffer, MAXBUFFER, ifp))
        {
          err = 1;
          if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
          break;
        }
        n++;
      } while (buffer[0] != '\r' && buffer[0] != '\n');*/
      if (err) break;
      
    }
    
  } while (!done);
  
  if (!err)
  {
    do
    {
      if (!fgets(buffer, MAXBUFFER, ifp))
      {
        err = 1;
        if (!feof(ifp)) fprintf(stderr, "Error reading from file!\n");
        break;
      }
      if (buffer[0]>=' ' && buffer[1]>=' ')
      {
        for (i=0; buffer[i]!=0; i++)
        {
          if (buffer[i]=='<') intag++;
          else if (buffer[i]=='>' && intag > 0) intag--;
          else if (!intag)
          {
            fprintf(ofp, "%c", buffer[i]);
          }
        }
      }
    } while (buffer[0] != '\r' && buffer[0] != '\n');
  }
  if (err && !feof(ifp))
  {
    return -1;
  }
  if (done) return 1;
  return 2;
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("SubExtractor - Extracts the correct subtitles from an SRT or a VTT file\n");
    printf("2019 DHeadshot's Software Creations\n");
    printf("Usage:\n  %s <SubFile> <OutFile> <TimeStamp> <SubType>\n", argv[0]);
    printf("Where:\n  <SubFile> is the file containing subtitles.\n");
    printf("  <OutFile> is the file to output the extracted subtitles to.\n");
    printf("  <TimeStamp> is the timestamp of the subtitles to find, in the\n   format hh:mm:ss,iii, (where hh is hours, mm is minutes, ss is seconds and\n   iii is milliseconds).\n");
    printf("  <SubType> is either SRT or VTT depending on the type of subtitle file.\n");
    return 1;
  }
  
  FILE *ifp, *ofp;
  tstamp t;
  memset(&t, 0, sizeof(tstamp));
  char stype = toupper(argv[4][0]);
  
  if (sscanf(argv[3], "%u:%u:%u,%u", 
             &(t.h), &(t.m), &(t.s), &(t.mil)) != 4)
  {
    if (sscanf(argv[3], "%u:%u:%u.%u", 
               &(t.h), &(t.m), &(t.s), &(t.mil)) != 4)
    {
      fprintf(stderr, "Error: Invalid format \"%s\"!\n", argv[3]);
      return 2;
    }
  }
  
  ifp = fopen(argv[1], "r");
  if (!ifp)
  {
    fprintf(stderr, "Error opening file \"%s\"!\n", argv[1]);
    return 3;
  }
  
  ofp = fopen(argv[2], "w");
  if (!ofp)
  {
    fprintf(stderr, "Error opening file \"%s\"!\n", argv[2]);
    fclose(ifp);
    return 4;
  }
  
  int ret;
  if (stype == 'S')  ret = outputsrtsubs(ifp, ofp, &t);
  else if (stype == 'V')  ret = outputvttsubs(ifp, ofp, &t);
  else
  {
    fprintf(stderr, "Unknown subtitles type '%s'!  Must be 'SRT' or 'VTT'!\n", argv[4]);
    fclose(ifp);
    fclose(ofp);
    return 5;
    
  }
  if (ret < 1)
  {
    fclose(ifp);
    fclose(ofp);
    return 6;
  }
  if (ret != 1) printf("No subtitles found for this timestamp.\n");
  
  fclose(ifp);
  fclose(ofp);
  return 0;
}
