#include "object.h"
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

object CLASS(File);
object CLASS(FileNotFoundError);
object CLASS(IOError);

// Instantiate a File with a filename
iridium_method(File, initialize) {
  object self = local("self");
  object filename = local("filename");
  object mode = local("mode");
  FILE * f = fopen(C_STRING(filename), C_STRING(mode));
  send(self, "__set__", ATOM("filename"), filename);
  send(self, "__set__", ATOM("mode"), mode);
  if (NULL == f) {
    handleException(send(CLASS(FileNotFoundError), "new", filename));
  }
  internal_set_attribute(self, ATOM("FILE"), f);
  return NIL;
}

char * read_file(FILE * f, object filename) {
  char * buffer;
  struct stat st;
  size_t nbytes_read;
  object reason;
  if (-1 == fstat(fileno(f), &st)) {
    reason = filename;
    reason = send(reason, "__add__", IR_STRING(" -- "));
    reason = send(reason, "__add__", IR_STRING(strerror(errno)));
    handleException(send(CLASS(IOError), "new", reason));
  }
  // Is +1 needed for a terminating null?
  buffer = GC_MALLOC((st.st_size+1)*sizeof(char));
  nbytes_read = fread(buffer, sizeof(char), st.st_size, f);
  if (nbytes_read != st.st_size) {
    // DEBUG
    printf("%lu != %lld\n", nbytes_read, st.st_size);
    handleException(send(CLASS(IOError), "new", IR_STRING("Reading Error")));    
  }
  buffer[st.st_size] = 0; // Add terminating NULL
  return buffer;
}

iridium_method(File, read) {
  object self = local("self");
  FILE * f = internal_get_attribute(self, ATOM("FILE"), FILE *);
  char * buffer = read_file(f, local("filename"));
  return IR_STRING(buffer);
}

iridium_method(File, close) {
  object self = local("self");
  object reason;
  
  FILE * f = internal_get_attribute(self, ATOM("FILE"), FILE *);
  if (f == NULL) {
    reason = send(self, "to_s");
    reason = send(reason, "__add__", IR_STRING(" is not open"));
    handleException(send(CLASS(IOError), "new", reason));  
  }
  fclose(f);
  return NIL;
}

iridium_classmethod(File, read) {
  object self = local("self");
  object filename = local("filename");
  object mode = IR_STRING("r");
  // Create a new file object
  object f = send(self, "new", filename, mode);
  // Delegate read to it
  object result = send(f, "read");
  // Close the file
  send(f, "close");
  // Return the read result
  return result;
}

void IR_init_File(void)
{
  CLASS(File) = send(CLASS(Class), "new", IR_STRING("File"));
  CLASS(FileNotFoundError) = send(CLASS(Class), "new", IR_STRING("FileNotFoundError"), CLASS(Exception));
  CLASS(IOError) = send(CLASS(Class), "new", IR_STRING("IOError"), CLASS(Exception));

  DEF_METHOD(CLASS(File), "initialize", ARGLIST(argument_new(ATOM("filename"), NULL, 0), argument_new(ATOM("mode"), IR_STRING("r"), 0)), iridium_method_name(File, initialize));
  DEF_METHOD(CLASS(File), "read", ARGLIST(), iridium_method_name(File, read));
  DEF_METHOD(CLASS(File), "close", ARGLIST(), iridium_method_name(File, close));
  DEF_FUNCTION(CLASS(File), "read", ARGLIST(argument_new(ATOM("filename"), NULL, 0)), iridium_classmethod_name(File, read));
}

