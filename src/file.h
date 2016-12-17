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
  send(self, "__set__", ATOM("filename"), filename);
  send(self, "__set__", ATOM("mode"), mode);
  return NIL;
}

char * read_file(char * path, char * mode) {
  char * buffer;
  struct stat st;
  FILE * f = fopen(path, mode);
  size_t nbytes_read;
  object reason;
  if (NULL == f) {
    handleException(send(CLASS(FileNotFoundError), "new", IR_STRING(path)));
  }
  if (-1 == fstat(fileno(f), &st)) {
    reason = IR_STRING(path);
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
  fclose(f);
  return buffer;
}

iridium_method(File, read) {
  object filename = local("filename"); // Defined on self
  object ir_mode = local("mode"); // Defined on self
  char * path = C_STRING(filename);
  char * mode = C_STRING(ir_mode);
  char * buffer = read_file(path, mode);
  return IR_STRING(buffer);
}

iridium_classmethod(File, read) {
  object self = local("self");
  object filename = local("filename");
  object mode = IR_STRING("r");
  // Create a new file object
  object f = send(self, "new", filename, mode);
  // Delegate read to it
  return send(f, "read");
}

void IR_init_File(void)
{
  CLASS(File) = send(CLASS(Class), "new", IR_STRING("File"));
  CLASS(FileNotFoundError) = send(CLASS(Class), "new", IR_STRING("FileNotFoundError"), CLASS(Exception));
  CLASS(IOError) = send(CLASS(Class), "new", IR_STRING("IOError"), CLASS(Exception));

  DEF_METHOD(CLASS(File), "initialize", ARGLIST(argument_new(ATOM("filename"), NULL, 0), argument_new(ATOM("mode"), IR_STRING("r"), 0)), iridium_method_name(File, initialize));
  DEF_METHOD(CLASS(File), "read", ARGLIST(), iridium_method_name(File, read));
  DEF_METHOD(CLASS(File), "read", ARGLIST(), iridium_method_name(File, read));
  DEF_FUNCTION(CLASS(File), "read", ARGLIST(argument_new(ATOM("filename"), NULL, 0)), iridium_classmethod_name(File, read));
}

