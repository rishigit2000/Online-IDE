#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 256

void edit_files(int lno, char* newln)
{
    const char *filename = "data.txt";
    char temp[] = "temp.txt";
    char str[MAX];
    FILE *fptr1, *fptr2;
    int linectr = 0;
    fptr1 = fopen(filename,"r");
    fptr2 = fopen(temp,"w");
    while (!feof(fptr1))
    {
        strcpy(str,"\0");
        fgets(str,MAX,fptr1);
        if (!feof(fptr1))
        {
            linectr++;
            if (linectr != lno)
                fprintf(fptr2,"%s",str);
            else
                fprintf(fptr2,"%s",newln);
         }
    }
    fclose(fptr1);
    fclose(fptr2);
    remove(filename);
    rename(temp,filename);
}

int main()
{
    char newln[] = "f\n";
    int lno = 6;
    edit_files(lno,newln);
    return 0;
}