#ifndef HELPER_METHODS
#define HELPER_METHODS

int isValidCommand(char* command, char* programsPath)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(programsPath);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
          if(strcmp(dir->d_name,command) == 0)
          {
            closedir(d);
            return(1);
          }
        }
    }
    return(0);
}

void removeNewLine(char* str)
{
  size_t ln = strlen(str) - 1;
  if (*str && str[ln] == '\n')
      str[ln] = '\0';
}


#endif
