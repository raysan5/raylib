/* Copyright (c) 2012, Kim Gräsman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of Kim Gräsman nor the names of contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL KIM GRÄSMAN BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "getopt.h"

#include <stddef.h>
#include <string.h>

const int no_argument = 0;
const int required_argument = 1;
const int optional_argument = 2;

char* optarg;
int optopt;
/* The variable optind [...] shall be initialized to 1 by the system. */
int optind = 1;
int opterr;

static char* optcursor = NULL;

/* Implemented based on [1] and [2] for optional arguments.
   optopt is handled FreeBSD-style, per [3].
   Other GNU and FreeBSD extensions are purely accidental.

[1] http://pubs.opengroup.org/onlinepubs/000095399/functions/getopt.html
[2] http://www.kernel.org/doc/man-pages/online/pages/man3/getopt.3.html
[3] http://www.freebsd.org/cgi/man.cgi?query=getopt&sektion=3&manpath=FreeBSD+9.0-RELEASE
*/
int getopt(int argc, char* const argv[], const char* optstring) {
  int optchar = -1;
  const char* optdecl = NULL;

  optarg = NULL;
  opterr = 0;
  optopt = 0;

  /* Unspecified, but we need it to avoid overrunning the argv bounds. */
  if (optind >= argc)
    goto no_more_optchars;

  /* If, when getopt() is called argv[optind] is a null pointer, getopt()
     shall return -1 without changing optind. */
  if (argv[optind] == NULL)
    goto no_more_optchars;

  /* If, when getopt() is called *argv[optind]  is not the character '-',
     getopt() shall return -1 without changing optind. */
  if (*argv[optind] != '-')
    goto no_more_optchars;

  /* If, when getopt() is called argv[optind] points to the string "-",
     getopt() shall return -1 without changing optind. */
  if (strcmp(argv[optind], "-") == 0)
    goto no_more_optchars;

  /* If, when getopt() is called argv[optind] points to the string "--",
     getopt() shall return -1 after incrementing optind. */
  if (strcmp(argv[optind], "--") == 0) {
    ++optind;
    goto no_more_optchars;
  }

  if (optcursor == NULL || *optcursor == '\0')
    optcursor = argv[optind] + 1;

  optchar = *optcursor;

  /* FreeBSD: The variable optopt saves the last known option character
     returned by getopt(). */
  optopt = optchar;

  /* The getopt() function shall return the next option character (if one is
     found) from argv that matches a character in optstring, if there is
     one that matches. */
  optdecl = strchr(optstring, optchar);
  if (optdecl) {
    /* [I]f a character is followed by a colon, the option takes an
       argument. */
    if (optdecl[1] == ':') {
      optarg = ++optcursor;
      if (*optarg == '\0') {
        /* GNU extension: Two colons mean an option takes an
           optional arg; if there is text in the current argv-element
           (i.e., in the same word as the option name itself, for example,
           "-oarg"), then it is returned in optarg, otherwise optarg is set
           to zero. */
        if (optdecl[2] != ':') {
          /* If the option was the last character in the string pointed to by
             an element of argv, then optarg shall contain the next element
             of argv, and optind shall be incremented by 2. If the resulting
             value of optind is greater than argc, this indicates a missing
             option-argument, and getopt() shall return an error indication.

             Otherwise, optarg shall point to the string following the
             option character in that element of argv, and optind shall be
             incremented by 1.
          */
          if (++optind < argc) {
            optarg = argv[optind];
          } else {
            /* If it detects a missing option-argument, it shall return the
               colon character ( ':' ) if the first character of optstring
               was a colon, or a question-mark character ( '?' ) otherwise.
            */
            optarg = NULL;
            optchar = (optstring[0] == ':') ? ':' : '?';
          }
        } else {
          optarg = NULL;
        }
      }

      optcursor = NULL;
    }
  } else {
    /* If getopt() encounters an option character that is not contained in
       optstring, it shall return the question-mark ( '?' ) character. */
    optchar = '?';
  }

  if (optcursor == NULL || *++optcursor == '\0')
    ++optind;

  return optchar;

no_more_optchars:
  optcursor = NULL;
  return -1;
}

/* Implementation based on [1].

[1] http://www.kernel.org/doc/man-pages/online/pages/man3/getopt.3.html
*/
int getopt_long(int argc, char* const argv[], const char* optstring,
  const struct option* longopts, int* longindex) {
  const struct option* o = longopts;
  const struct option* match = NULL;
  int num_matches = 0;
  size_t argument_name_length = 0;
  const char* current_argument = NULL;
  int retval = -1;

  optarg = NULL;
  optopt = 0;

  if (optind >= argc)
    return -1;

  if (strlen(argv[optind]) < 3 || strncmp(argv[optind], "--", 2) != 0)
    return getopt(argc, argv, optstring);

  /* It's an option; starts with -- and is longer than two chars. */
  current_argument = argv[optind] + 2;
  argument_name_length = strcspn(current_argument, "=");
  for (; o->name; ++o) {
    if (strncmp(o->name, current_argument, argument_name_length) == 0) {
      match = o;
      ++num_matches;
    }
  }

  if (num_matches == 1) {
    /* If longindex is not NULL, it points to a variable which is set to the
       index of the long option relative to longopts. */
    if (longindex)
      *longindex = (int) (match - longopts);

    /* If flag is NULL, then getopt_long() shall return val.
       Otherwise, getopt_long() returns 0, and flag shall point to a variable
       which shall be set to val if the option is found, but left unchanged if
       the option is not found. */
    if (match->flag)
      *(match->flag) = match->val;

    retval = match->flag ? 0 : match->val;

    if (match->has_arg != no_argument) {
      optarg = strchr(argv[optind], '=');
      if (optarg != NULL)
        ++optarg;

      if (match->has_arg == required_argument) {
        /* Only scan the next argv for required arguments. Behavior is not
           specified, but has been observed with Ubuntu and Mac OSX. */
        if (optarg == NULL && ++optind < argc) {
          optarg = argv[optind];
        }

        if (optarg == NULL)
          retval = ':';
      }
    } else if (strchr(argv[optind], '=')) {
      /* An argument was provided to a non-argument option.
         I haven't seen this specified explicitly, but both GNU and BSD-based
         implementations show this behavior.
      */
      retval = '?';
    }
  } else {
    /* Unknown option or ambiguous match. */
    retval = '?';
  }

  ++optind;
  return retval;
}
