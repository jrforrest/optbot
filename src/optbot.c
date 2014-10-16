#include <liboptbot.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

struct arg_params {
  char little;
  char* big;
  char* description;
  bool takes_value;
};

/* If any function returns failure, its error message should be kept here. */
char err_msg[256];

struct arg_params* init_arg_params(void) {
  struct arg_params* params = malloc(sizeof(struct arg_params));

  params->little = '\0';
  params->big = NULL;
  params->description=NULL;
  params->takes_value = false;

  return params;
}

#define validate(condition, err_str, ...) \
  if(! (condition)) { \
    sprintf((char*) &err_msg, err_str, ##__VA_ARGS__); \
    return NULL;\
  }

struct arg_params* parse_arg_params(char* csv_string) {
  char* str;
  struct arg_params* arg = init_arg_params();

  str = strtok(csv_string, ",");
  validate(strlen(str) == 1,
    "Single character expected for little opt, \"%s\" was given.", str);
  arg->little = str[0];

  str = strtok(NULL, ",");
  validate(str, "Big opt must be given for args!")
  validate(strlen(str) >= 2,
    "String longer than 1 char expected for big opt, \"%s\" was given.", str);
  validate(strlen(str) <= 64,
    "big opt may not be longer than 64 characters");
  arg->big = str;

  arg->description = strtok(NULL, ",");
  validate(arg->description, "Description must be given for args!");

  str = strtok(NULL, ",");
  validate(str, "takes_value boolean must be given for args!");
  if(strcmp("true", str) == 0) {
    arg->takes_value = true;
  } else {
    validate(strcmp("false", str) == 0, "Value expectation must be "
      "\"true\" or \"false\". \"%s\" was given.", str);
  }

  return arg;
}

void export_arg(struct cli_arg* arg, char* prefix) {
  int i;

  if(!prefix) prefix = "arg_";

  if(arg->values_length > 0) {
    printf("%s%s=%s;", prefix, arg->big, arg->values[0]);
  } else if(arg->times_set > 0){
    printf("%s%s=true;", prefix, arg->big);
  } else {
    printf("%s%s=false;", prefix, arg->big);
  }

  if(arg->values_length > 1) {
    printf("%s%s_n=%d;", prefix, arg->big, arg->values_length);
    for(i = 0; i < arg->values_length; i++) {
      printf("%s%s_%d=%s;", prefix, arg->big, i + 1, arg->values[i]);
    }
  }
}

void export_argv(struct cli_arg_list* parse_list, char* prefix) {
  int i;
  if(!prefix) prefix = "arg_";
  for(i = 1; i < parse_list->argc; i++)
    printf("%sargv_%d=%s;", prefix, i, parse_list->argv[i]);
  printf("%sargc=%d;", prefix, parse_list->argc - 1);
}

int main(int argc, const char** argv) {
  struct cli_arg_list* arg_list = init_cli_arg_list();
  struct cli_arg_list* parse_list = init_cli_arg_list();
  struct cli_arg* arg;
  struct arg_params** argp;
  bool fail_loudly = false;
  bool fail_hard = false;
  char* prefix = NULL;
  int i;

  add_arg(arg_list, 'a', "add_arg",
    "Adds an argument to be parsed.\n\n"
    "    A comma value list should be given to each instance of this option\n"
    "    the values are as follows: little,big,description,takes_value.\n"
    "      little: The single character option for the parameter\n"
    "      big: The long option for the parameter.  this is also used as the\n"
    "           name of the environment variable that will be set if this\n"
    "           parameter is given\n"
    "      description:  The description for the argument that should show up\n"
    "                    in the help display\n\n"
    "      takes_value:  Must be one of \"true\" or \"false\".  Determines\n"
    "                    whether or not optbot will expect a value to be \n"
    "                    given for this option.  Any values given to the \n"
    "                    option will be stored in the parameter names with \n"
    "                    the big option string, with a number appended for\n"
    "                    each successive value.",
    true
  );

  add_arg(arg_list, 'p', "variable_prefix",
    "The prefix that should be appended to exported env variables\n"
    "    May not be longer than 16 characters.",
    true
  );

  add_arg(arg_list, 'h', "help", "Prints this help", false);
  add_arg(arg_list, 'd', "delimiter",
    "The character used to seperate arguments to --add-arg",
    false
  );
  add_arg(arg_list, 'f', "fail_var",
    "Prints arg_success=false; on failure.  Should be used when output\n"
    "   of this program is being eval'd.",
    false
  );

  add_arg(arg_list, 'e', "exit_on_failure",
    "Causes an exit command to be printed on failure.  Should be used when\n"
    "    output of this program is being eval'd.",
    false
  );

  /* Note that this option is provided on the parse_list */
  add_arg(parse_list, 'h', "help", "Show this help page", false);

  arg_list->devour_flag = true;

  if(! parse_command_line(arg_list, argc, argv)) {
    fprintf(stderr, "optbot failure: %s\n", arg_list->message);
    goto error;
  }

  if(little_opt_arg(arg_list, 'h')->times_set) {
    print_help(arg_list);
    goto error;
  }

  if(little_opt_arg(arg_list, 'f')->times_set)
    fail_loudly = true;

  if(little_opt_arg(arg_list, 'e')->times_set)
    fail_hard = true;

  arg = little_opt_arg(arg_list, 'p');
  if(arg->times_set > 1) {
    fprintf(stderr, "optbot failure: Only one variable prefix is allowed!\n");
    goto error;
  } else if(arg->times_set == 1) {
    prefix = strdup(arg->values[0]);
    checkmem(prefix);
  }

  arg = little_opt_arg(arg_list, 'a');
  argp = malloc(sizeof(struct arg_params*) * arg->values_length);
  checkmem(argp)
  for(i = 0; i < arg->values_length; i++) {
    argp[i] = parse_arg_params(arg->values[i]);

    if(!argp[i]) {
      fprintf(stderr, "%s\n", err_msg);
      goto error;
    }

    add_arg(parse_list, argp[i]->little, argp[i]->big,
      argp[i]->description, argp[i]->takes_value);
  }

  if(! parse_command_line(parse_list, arg_list->argc,
    (const char**)arg_list->argv))
  {
    fprintf(stderr, "%s\n", parse_list->message);
    goto error;
  }

  /* Make it possible to print the help for the script calling this by
   * passing it -h or --help */
  if(little_opt_arg(parse_list, 'h')->times_set > 0) {
    print_help(parse_list);
    goto error;
  }

  export_argv(parse_list, prefix);

  for(i = 0; i < arg->values_length; i++)
    export_arg(little_opt_arg(parse_list, argp[i]->little), prefix);

  return 0;

  error:
    if(fail_loudly) printf("arg_success=false;");
    if(fail_hard) printf("exit 1;");
    return false;
}
