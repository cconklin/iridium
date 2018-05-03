#include "ir_regex.h"

iridium_method(Regex, initialize) {
  object self = local("self");
  object pattern = local("pattern");

  char * err;
  int err_offset;

  const int INFO_CAPTURECOUNT = 2;

  int captures;
  pcre_extra * extra;
  pcre * code;

  // Compile and get information about the regex

  code = pcre_compile(C_STRING(pattern), 0, (const char **) &err, &err_offset, NULL);

  // pcre_compile returns NULL on error and sets err and err_offset
  if (code == NULL) {
    object reason = IR_STRING("could not compile '");
    send(reason, "__add__", pattern);
    send(reason, "__add__", IR_STRING("': "));
    send(reason, "__add__", IR_STRING(err));
    handleException(send(CLASS(ArgumentError), "new", reason));
  }

  extra = pcre_study(code, 0, (const char **) &err);

  if (extra == NULL) {
    object reason = IR_STRING("could not study '");
    send(reason, "__add__", pattern);
    send(reason, "__add__", IR_STRING("': "));
    send(reason, "__add__", IR_STRING(err));
    send(reason, "__add__", IR_STRING(" at "));
    send(reason, "__add__", _send(FIXNUM(err_offset), "to_s", 0));
    handleException(send(CLASS(ArgumentError), "new", reason));
  }

  pcre_fullinfo(code, NULL, INFO_CAPTURECOUNT, &captures);

  // Store regex info into the object
  send(self, "__set__", ATOM("pattern"), pattern);
  internal_set_attribute(self, ATOM("code"), code);
  internal_set_attribute(self, ATOM("extra"), extra);
  internal_set_integral(self, ATOM("captures"), captures);

  return NIL;
}

iridium_method(Regex, match) {
  object self = local("self");
  object str = local("str");
  object pos_or_fn = local("pos_or_fn");
  object fun = local("fun");
  object pos;
  if (isA(pos_or_fn, CLASS(Integer))) {
    pos = pos_or_fn;
  } else if (isA(pos_or_fn, CLASS(Function))) {
    fun = pos_or_fn;
    pos = FIXNUM(0);
  } else {
    handleException(send(CLASS(ArgumentError), "new", IR_STRING("pos must be an integer")));
    return NIL;
  }

  pcre * code = internal_get_attribute(self, ATOM("code"), pcre *);
  pcre_extra * extra = internal_get_attribute(self, ATOM("extra"), pcre_extra *);

  int captures = internal_get_integral(self, ATOM("captures"), size_t);
  size_t ovector_size = (captures + 1) * 3;
  int * ovector = GC_MALLOC(ovector_size);
  int ret;

  object matchdata;

  assert(ovector);

  if (INT(pos) > INT(send(str, "size"))) {
    return NIL;
  }

  const char * cstr = C_STRING(str);

  ret = pcre_exec(code, extra, cstr, strlen(cstr), INT(pos), 0, ovector, ovector_size);

  if (ret >= 0) {
    matchdata = create_matchdata(self, code, str, INT(pos), ovector, captures);
    return calls(fun, array_push(array_new(), matchdata));
  } else {
    return NIL;
  }
}

// new and initialize are NOT defined on MatchData since it needs to be passed non-iridiium objects
object create_matchdata(object regex, pcre * code, object string, int pos, int * ovector, int captures) {
  object matchdata = construct(CLASS(MatchData));
  set_attribute(matchdata, ATOM("regex"), PUBLIC, regex);
  set_attribute(matchdata, ATOM("string"), PUBLIC, string);
  internal_set_attribute(matchdata, ATOM("code"), code);
  internal_set_attribute(matchdata, ATOM("ovector"), ovector);
  internal_set_integral(matchdata, ATOM("pos"), pos);
  internal_set_integral(matchdata, ATOM("captures"), captures);
  return matchdata;
}

iridium_method(MatchData, __get_index__) {
  object self = local("self");
  object index = local("index");
  pcre * code;
  int idx = -1;

  object string = get_attribute(self, ATOM("string"), PUBLIC);
  int * ovector = internal_get_attribute(self, ATOM("ovector"), int *);
  int start;
  int finish;

  char * str = C_STRING(string);
  char * match;

  if (isA(index, CLASS(Integer))) {
    idx = INT(index);
  } else if (isA(index, CLASS(String))) {
    code = internal_get_attribute(self, ATOM("code"), pcre *);
    idx = pcre_get_stringnumber(code, C_STRING(index));
  } else {
    handleException(send(CLASS(ArgumentError), "new", IR_STRING("index must be a Integer or String")));
  }

  if (validGroup(self, idx)) {
    start = ovector[idx * 2];
    finish = ovector[idx * 2 + 1];
    if (start < 0) {
      return NIL;
    }
    match = GC_MALLOC((finish - start + 1) * sizeof(char));
    strncpy(match, str + start, finish - start);
    // Ensure the last character is the null terminator
    match[finish - start] = 0;
    return IR_STRING(match);
  }

  return NIL;
}

int validGroup(object match_data, int group) {
  int captures = internal_get_integral(match_data, ATOM("captures"), int);
  return group <= captures;
}

iridium_method(Lambda, match_default_fn) {
  return local("match");
}

void IR_init_Regex(void) {

  object match_lambda = FUNCTION(ATOM("lambda"), ARGLIST(argument_new(ATOM("match"), NULL, 0)), dict_new(ObjectHashsize), iridium_method_name(Lambda, match_default_fn));

  CLASS(Regex) = send(CLASS(Class), "new", IR_STRING("Regex"));
  DEF_METHOD(CLASS(Regex), "initialize", ARGLIST(argument_new(ATOM("pattern"), NULL, 0)), iridium_method_name(Regex, initialize));
  DEF_METHOD(CLASS(Regex), "match", ARGLIST(argument_new(ATOM("str"), NULL, 0), argument_new(ATOM("pos_or_fn"), FIXNUM(0), 0), argument_new(ATOM("fun"), match_lambda, 0)), iridium_method_name(Regex, match));

  CLASS(MatchData) = send(CLASS(Class), "new", IR_STRING("Regex.MatchData"));
  DEF_METHOD(CLASS(MatchData), "__get_index__", ARGLIST(argument_new(ATOM("index"), NULL, 0)), iridium_method_name(MatchData, __get_index__));

  no_attribute(CLASS(MatchData), ATOM("new"));
  no_instance_attribute(CLASS(MatchData), ATOM("initialize"));

  define_constant(ATOM("Regex"), CLASS(Regex));
}

