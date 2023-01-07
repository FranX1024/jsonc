#include <stdio.h>
#include <json.h>

int main() {
  char * buffer = 0;
  uint length;
  FILE * f = fopen ("test.json", "rb");

  if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length);
      if (buffer)
	{
	  fread (buffer, 1, length, f);
	}
      fclose (f);
    }

  if (buffer)
    {
      json_t* obj = json_parse(buffer);
      json_t* studs = json_get_str(obj, "students");
      if(studs == 0) {
	printf("Could not get student list!\n");
	json_drop(obj);
	free(buffer);
	return 0;
      }
      printf("----------------------------\n");
      for(uint i = 0; i < studs->value.arr->size; i++) {
	json_t* stud = json_get_num(studs, i);
	if(stud == 0) continue;
	json_t* name = json_get_str(stud, "name");
	json_t* grades = json_get_str(stud, "grades");
	if(name != 0 && name->type == JSON_STR)
	  printf("NAME: %s\n", name->value.str);
	else
	  printf("UNNAMED STUDENT\n");
	if(grades != 0 && grades->type == JSON_ARR) {
	  printf("GRADES: ");
	  for(uint j = 0; j < grades->value.arr->size; j++) {
	    json_t* grade = json_get_num(grades, j);
	    if(grade != 0 && grade->type == JSON_NUM)
	      printf("%d ", (int)grade->value.num);
	  }
	  printf("\n");
	}
	else printf("STUDENT HAS NO GRADES\n");
	printf("----------------------------\n");
      }
      json_drop(obj);
      free(buffer);
    }
  else printf("ERR: could not open file!\n");

  return 0;
}
