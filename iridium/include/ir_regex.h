#include "ir_object.h"
#include <pcre.h>
#pragma once

object CLASS(Regex);
object CLASS(MatchData);

void IR_init_Regex(struct IridiumContext *);

iridium_method(Regex, initialize);
iridium_method(Regex, match);

iridium_method(MatchData, __get_index__);

object create_matchdata(object, pcre*, object, int, int*, int);

int validGroup(object, int);

