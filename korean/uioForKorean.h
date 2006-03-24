#include 
class io_UNICODE_Text;
{
    FILE *f;
public:
   io_UNICODE_Text(){
      f = 0;
   }
   ~io_UNICODE_Text(){
     if(f) fclose(f);
   };
   void u_TextCree(char *f){
   }
   FILE *getFilePtr()
   {
      return(f);
   }
};
