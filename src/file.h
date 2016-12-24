#include "object.h"
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#pragma once

object CLASS(File);
object CLASS(FileNotFoundError);
object CLASS(IOError);

void IR_init_File(void);
