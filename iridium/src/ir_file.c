#include "ir_file.h"
#include "atoms.h"

FILE * get_file(struct IridiumContext * context, object self) {
  object reason = NULL;
  FILE * f = internal_get_attribute(self, L_ATOM(FILE), FILE *);
  if (f == NULL) {
    reason = send(self, "to_s");
    reason = send(reason, "__add__", IR_STRING(" is not open"));
    RAISE(send(CLASS(IOError), "new", reason));  
  }
  return f;
}

size_t file_length(struct IridiumContext * context, FILE * f, object filename) {
  struct stat st;
  object reason = NULL;
  if (-1 == fstat(fileno(f), &st)) {
    reason = filename;
    reason = send(reason, "__add__", IR_STRING(" -- "));
    reason = send(reason, "__add__", IR_STRING(strerror(errno)));
    RAISE(send(CLASS(IOError), "new", reason));
  }
  return st.st_size;
}

// Instantiate a File with a filename
iridium_method(File, initialize) {
  object self = local(self);
  object filename = local(filename);
  object mode = local(mode);
  FILE * f = fopen(C_STRING(context, filename), C_STRING(context, mode));
  send(self, "__set__", L_ATOM(filename), filename);
  send(self, "__set__", L_ATOM(mode), mode);
  if (NULL == f) {
    RAISE(send(CLASS(FileNotFoundError), "new", filename));
  }
  internal_set_attribute(self, L_ATOM(FILE), f);
  return NIL;
}

char * read_file(struct IridiumContext * context, FILE * f, object filename) {
  char * buffer = NULL;
  size_t file_size;
  size_t nbytes_read;
  file_size = file_length(context, f, filename);
  // Is +1 needed for a terminating null?
  buffer = GC_MALLOC((file_size+1)*sizeof(char));
  nbytes_read = fread(buffer, sizeof(char), file_size, f);
  if (nbytes_read != file_size) {
    // DEBUG
    printf("%lu != %zu\n", nbytes_read, file_size);
    RAISE(send(CLASS(IOError), "new", IR_STRING("Reading Error")));    
  }
  buffer[file_size] = 0; // Add terminating NULL
  return buffer;
}

iridium_method(File, read) {
  object self = local(self);
  FILE * f = get_file(context, self);
  char * buffer = read_file(context, f, local(filename));
  return IR_STRING(buffer);
}

iridium_method(File, write) {
  // The string to write
  char * str = C_STRING(context, local(str));
  size_t len = strlen(str);
  FILE * f = internal_get_attribute(local(self), L_ATOM(FILE), FILE *);
  size_t written = fwrite(str, sizeof str[0], len, f);
  if (written != len) {
    RAISE(send(CLASS(IOError), "new", IR_STRING("File not in write mode")));
  }
  return NIL;
}

iridium_method(File, close) {
  object self = local(self);
  
  FILE * f = get_file(context, self);
  fclose(f);
  // Now that the file has been closed, set it to NULL
  internal_set_attribute(self, L_ATOM(FILE), NULL);
  return NIL;
}

iridium_method(File, each_line) {
  object self = local(self);
  object filename = local(filename); // From self
  object fn = local(fn);
  object str = NULL;
  FILE * f = get_file(context, self);
  size_t file_size = file_length(context, f, filename);
  char * buffer = GC_MALLOC((file_size+1)*sizeof(char));
  assert(buffer);
  int nchars;
  char * line = NULL;

  while ((nchars = getline(&buffer, &file_size, f)) != -1) {
    // Remove the newline, if present
    if (buffer[nchars-1] == '\n') {
      buffer[nchars-1] = 0;
    }
    line = GC_MALLOC((nchars + 1) * sizeof(char));
    assert(line);
    strncpy(line, buffer, nchars);
    str = IR_STRING(line);
    calls(context, fn, array_push(array_new(), str));
  }
  return NIL;
}

iridium_classmethod(File, read) {
  object self = local(self);
  object filename = local(filename);
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

void IR_init_File(struct IridiumContext * context)
{
  CLASS(File) = send(CLASS(Class), "new", IR_STRING("File"));
  CLASS(FileNotFoundError) = send(CLASS(Class), "new", IR_STRING("FileNotFoundError"), CLASS(Exception));
  CLASS(IOError) = send(CLASS(Class), "new", IR_STRING("IOError"), CLASS(Exception));

  DEF_METHOD(CLASS(File), "initialize", ARGLIST(argument_new(L_ATOM(filename), NULL, 0), argument_new(L_ATOM(mode), IR_STRING("r"), 0)), iridium_method_name(File, initialize));
  DEF_METHOD(CLASS(File), "read", ARGLIST(), iridium_method_name(File, read));
  DEF_METHOD(CLASS(File), "write", ARGLIST(argument_new(L_ATOM(str), NULL, 0)), iridium_method_name(File, write));
  DEF_METHOD(CLASS(File), "close", ARGLIST(), iridium_method_name(File, close));
  DEF_FUNCTION(CLASS(File), "read", ARGLIST(argument_new(L_ATOM(filename), NULL, 0)), iridium_classmethod_name(File, read));
  DEF_METHOD(CLASS(File), "each_line", ARGLIST(argument_new(L_ATOM(fn), NULL, 0)), iridium_method_name(File, each_line));

  define_constant(L_ATOM(File), CLASS(File));
}
