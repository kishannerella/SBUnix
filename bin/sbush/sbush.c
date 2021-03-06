#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


#define MAX_BUFFER_SIZE    1024
#define MAX_ARG_COUNT      100
#define MAX_ARG_SIZE       100
#define MAX_PIPE_COUNT     100
#define MAX_PIPE_CMD_SIZE  MAX_ARG_SIZE


void sbuprintmsg(char *str)
{
   fputs(str, stdout);
}

void sbuprintline(char* str)
{
   sbuprintmsg(str);
   sbuprintmsg("\n");
}

void sbuerr(char *str)
{
   sbuprintline(str);
}

int sbustrncmp(char *str1, char *str2, int size)
{
   int i;

   if (str1 == NULL && str2 == NULL)
      return 1;

   if (str1 == NULL || str2 == NULL)
      return 0;

   for (i = 0; i < size;i++)
   {
      if (*(str1 + i) != *(str2 + i))
         return 0; 
   }

   return 1;
}

int sbustrlen(char *str)
{
   char *c = str;
   int   len = 0;

   if (!str)
      return -1;

   while (*c != '\0')
   {
      c++;
      len++;
   }

   return len;
}

int iscmd(char *buf, char *cmd)
{
   return sbustrncmp(buf, cmd, sbustrlen(cmd));
}

void sbustrncpy(char* dest, char* src, int len)
{
   int i;
  
   for (i = 0;i < len && src[i];i++)
   {
      dest[i] = src[i];
   }

   if (i != len)
   {
      *(dest + i) = '\0';
   }
}

/********************** sbusplit ************************/
/* sbusplit - This function splits the input buffer using delimiter.
 *
 * Arguments :
 * buf       - input string
 * args      - the output set of strings
 * delimiter - splits the string based on delimiter
 * Returns the number of arguments
 */
int sbusplit(char* buf, char args[][MAX_ARG_SIZE], char delimiter)
{
   char  *c = buf;
   char  *ch;
   int    argcnt = 0;
   int    i;
   char  *argstart, *argend;
 
   while (*c != '\0')
   {
      while (*c == ' ')
         c++;
      argstart = c;

      while (*c != delimiter  && *c != '\0')
         c++;
      argend = c;

      for (i=0, ch = argstart; ch < argend; ch++, i++)
         args[argcnt][i] = *ch;
      args[argcnt][i] = '\0';

      argcnt++;
      if (argcnt >= MAX_ARG_COUNT)
      {
         sbuerr("error - too many arguments");
         break;
      }

      /* If we reached the end of string, break.
       * else go to the next character */
      if (!*c)
         break;
      else
         c++;
   }
   return argcnt;
}

int runcmd(char *buf)
{
   char *c = buf;
   
   /* Consider EOF as well */ 
   while( *c == ' ' || *c == '\t')
   {
      c++;
   }

   if (*c == '\0')
   {
      return 0;
   }

   if (iscmd(c, "#")) 
   {
      return 0;
   }
   else if ((iscmd(c, "pwd") && !*(c+3))||
        iscmd(c, "pwd "))
   {
      char cwd[MAX_BUFFER_SIZE];
      if (getcwd(cwd, sizeof(cwd)))
      {
         sbuprintline(cwd);
      }
      else
      {
         sbuprintline("error - unable to get present working directory");
      }
   }
   /* (exit is matched and we are at the end of the buffer) OR 
    * (exit is matched and there is a space next */
   else if ((iscmd(c, "exit") && !*(c+4))||
            iscmd(c, "exit "))
   {
      return 1; 
   }
   else if (iscmd(c, "cd "))
   {
      char *pathstart;
      char *pathend;
      char  path[MAX_BUFFER_SIZE];
      char *ch;
      int   i;

      /* Extract the path out of the buffer by doing the following.
       * 0. Start with the character after cd
       * 1. Search for the first non space character. Mark it as pathstart
       * 2. Search for the first space or the end of buf after pathstart.
       *    Mark it as pathend
       * 3. Copy [pathstart, pathend) to 'path' array. Now call chdir
       */
      c += sbustrlen("cd ");
      while ( *c == ' ' || *c == '\t')
         c++;
      pathstart = c;

      while ( *c != ' ' && *c != '\t' && *c != '\0')
         c++;
      pathend = c;

      for ( i=0, ch = pathstart; ch < pathend; ch++, i++)
         path[i] = *ch;
      path[i] = '\0';
   
      if (chdir(path))
      {
         sbuerr("error - unable to change directory");
      } 
   }
   else if (iscmd(c, "export "))
   {
      char *keystart;
      c += sbustrlen("export ");

      while (*c == ' ' || *c == '\t')
         c++;
      keystart = c;

      while (*c != '=' && *c != '\0')
         c++;

      /* incomplete command - 'export PATH' */
      if (*c == '\0')
         return 0;
      else
      {
         /* *c == '=' */
         *c = '\0';
      }

      if (setenv(keystart, c+1, 1))
      {
         sbuerr("error - unable to set environment variable");
      }
      //printf("%s\n", getenv("PATH"));
      //printf("%s\n", getenv("PS1"));
   }
   else
   {
      char  pipeargs[1][MAX_PIPE_CMD_SIZE];
      char *args[1][MAX_ARG_COUNT];
      char  argscontent[1][MAX_ARG_COUNT][MAX_ARG_SIZE];
      int   fd[MAX_PIPE_COUNT-1][2];
      int   background = 0;             
      int   pid = 0;
      int   status;
      int   argcount;
      int   pipeargcount;                     // # of pipes + 1 
      int   i, j;

      pipeargcount = sbusplit(c, pipeargs, '|');

      for (i = 0;i < pipeargcount;i++)
      { 
         argcount = sbusplit(c, argscontent[i],' ');
         for (j = 0;j < argcount;j++)
            args[i][j] = argscontent[i][j];

         // If the command needs be run in the background, the last argument
         // * should be '&' and we are not supposed to pass that to execvp*()
         // * If '&' is not set, we make the LAST argument as NULL. 
         if (pipeargcount == 1 &&   // no pipes 
             argcount > 0 && 
             iscmd(argscontent[i][argcount-1], "&"))
         {
            background = 1;
            args[i][argcount-1] = NULL;
         }
         else
         {
            args[i][argcount] = NULL;
         }
      }

      // We don't support pipe and background task in the same command 
      if (pipeargcount > 1 && background)
      {
         sbuerr("error - invalid pipe and background combination");
         return 0;
      }

      for (i = 0;i < pipeargcount - 1;i++)
      {
         if (pipe(fd[i]))
         {
            sbuerr("error - invalid pipe and background combination");
            return 0;
         }
      }

      for (i = 0;i < pipeargcount;i++)
      {
         pid = fork();
         if (pid == 0)
         {
            if (i > 0)
            {
               if (dup2(fd[i-1][0], 0) < 0)
                  sbuerr("error - dup2 failed");
            }

            if (i < pipeargcount-1)
            {
               if (dup2(fd[i][1], 1) < 0)
                  sbuerr("error - dup2 failed");
            }

            for (j = 0;j < pipeargcount - 1;j++)
            {
               close(fd[j][0]);
               close(fd[j][1]);
            }
            if (execvp(args[i][0], args[i]))
            {
               sbuerr("error - invalid command/unable to execute");
            }
 
            exit(1);
         }
      }

      if (pid < 0)
      {
         sbuerr("error - fork failed");
         return 0;
      }
      else if (pid > 0)
      {
         for (i = 0;i < pipeargcount - 1;i++)
         {
            close(fd[i][0]);
            close(fd[i][1]);
         }

         if (!background)
         {
            for (i = 0;i < pipeargcount;i++)
               waitpid(pid, &status);
         }
      }
   }

   return 0;
}


void print_prompt()
{
   char buf[200] = "sbush:";
   int len;
   getcwd(buf+sbustrlen(buf), 200);
   len = sbustrlen(buf);
   buf[len] = '$';
   buf[len+1] = ' ';
   buf[len+2] = '\0';
  
   sbuprintmsg(buf);
}

int main(int argc, char *argv[], char *envp[])
{
   char  buffer[MAX_BUFFER_SIZE];
   char *c;
   FILE *fp = stdin;

   if (argc > 1)
   {
      fp = fopen(argv[1], "r");
   }

   setenv("PS1", "sbush>", 1);
   while(1)
   {
      if (argc == 1)
         print_prompt();

      /* fgets stores '\n' AND '\0' at the end of the buffer unlike
       * gets which stores only '\0'. So, we find '\n' and replace
       * it with '\0' before starting to process to make our lives easier.
       * We are going back and forth between fgets and gets. You may
       * have to uncomment the following code.
       */
      if (fgets(buffer, MAX_BUFFER_SIZE, (argc > 1 )? fp:stdin) < 0)
      {
         break;
      }

      c = buffer;
      while (*c != '\n' && *c != '\0')
         c++;
      *c = '\0';

      if (runcmd(buffer))
      {
         break;
      }
   }

   if (argc == 1)
      puts("Bye...");
   fclose(fp); 
   return 0;
}
