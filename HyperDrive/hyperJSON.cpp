#include "hyperJSON.h"

static bool get_data_point_from_contents_int(const JsonArrayConst& contents, const char* module_name, const char* data_key, int& out) {
  for (const JsonVariantConst& module_data : contents) {

    const char* name = module_data["module"];
    if (name == nullptr){
      out = 1;
      continue;
    }

    if (strncmp(name, module_name, 15) != 0){
      out = 5;
    
      continue;
    }

    const JsonObjectConst data_obj = module_data["data"];

    if (data_obj.isNull()){
      out = 10;
    
      continue;
    }
      

    const JsonVariantConst data_value = data_obj[data_key];
    if (!data_value.isNull() && data_value.is<int>()) {

      out = data_value.as<int>();

      return true;
    }
  }
  return false;
}


void json_to_struct(const JsonObjectConst& data, hyper_Base& out) {
  // name, instance
  const JsonObjectConst id = data["id"];
  if (!id.isNull()) {
    const char* name = id["name"];
    if (name != nullptr)
      strncpy(out.data.name, name, 15);
    else
      out.data.name[0] = 0;
    out.data.instance = id["instance"] | -1;
  }

  // get a reference to the "contents" object
  const JsonArrayConst contents = data["contents"];
  if (contents.isNull())
    return;
  // find every data point we care about, and populate the struct with it

  get_data_point_from_contents_int(contents, "Hyper", "B", out.data.Pass);

  get_data_point_from_contents_int(contents, "X_Location", "MM", out.data.X);

  get_data_point_from_contents_int(contents, "Y_Location", "MM", out.data.Y);

  get_data_point_from_contents_int(contents, "Z_Location", "MM", out.data.Z);

  get_data_point_from_contents_int(contents, "MaxSpeed", "Velocity", out.data.MS);

  get_data_point_from_contents_int(contents, "Spool_Rad_X", "Radius", out.data.SRX);

  get_data_point_from_contents_int(contents, "Spool_Rad_YZ", "Radius", out.data.SRYZ);

  get_data_point_from_contents_int(contents, "GoTo", "B", out.data.Go);

  get_data_point_from_contents_int(contents, "Loop", "B", out.data.L);

  get_data_point_from_contents_int(contents, "Reset", "B", out.data.R);

  get_data_point_from_contents_int(contents, "Calibrate", "B", out.data.C);

  get_data_point_from_contents_int(contents, "PeriodX", "Num", out.data.PX);

  get_data_point_from_contents_int(contents, "PeriodY", "Num", out.data.PY);

  get_data_point_from_contents_int(contents, "PeriodZ", "Num", out.data.PZ);

}

static JsonObject make_module_object(const JsonArray& contents, const char* name) {
  const JsonObject data = contents.createNestedObject();
  data["module"] = name;
  return data.createNestedObject("data");
}

void struct_to_json(const hyper_Base& in, const JsonObject& out) {
  // start adding object to it!
  out["type"] = "data";
  // id block
  const JsonObject id = out.createNestedObject("id");
  id["name"] = in.data.name;
  id["instance"] = in.data.instance;
  // contents array
  const JsonArray contents = out.createNestedArray("contents");
  // add objects for each sensor!

  {
    const JsonObject data = make_module_object(contents, "Hyper");
    data["B"] = in.data.Pass;
  }
  {
    const JsonObject data = make_module_object(contents, "X_Location");
    data["MM"] = in.data.X;
  }
  {
    const JsonObject data = make_module_object(contents, "Y_Location");
    data["MM"] = in.data.Y;
  }
  {
    const JsonObject data = make_module_object(contents, "Z_Location");
    data["MM"] = in.data.Z;
  }
  {
    const JsonObject data = make_module_object(contents, "MaxSpeed");
    data["Velocity"] = in.data.MS;
  }
  {
    const JsonObject data = make_module_object(contents, "Spool_Rad_X");
    data["Radius"] = in.data.SRX;
  }
  {
    const JsonObject data = make_module_object(contents, "Spool_Rad_YZ");
    data["Radius"] = in.data.SRYZ;
  }
  {
    const JsonObject data = make_module_object(contents, "GoTo");
    data["B"] = in.data.Go;
  }
  {
    const JsonObject data = make_module_object(contents, "Loop");
    data["B"] = in.data.L;
  }
  {
    const JsonObject data = make_module_object(contents, "Reset");
    data["B"] = in.data.R;
  }
  {
    const JsonObject data = make_module_object(contents, "Calibrate");
    data["B"] = in.data.C;
  }
  {
    const JsonObject data = make_module_object(contents, "PeriodX");
    data["Num"] = in.data.PX;
  }
  {
    const JsonObject data = make_module_object(contents, "PeriodY");
    data["Num"] = in.data.PY;
  }
  {
    const JsonObject data = make_module_object(contents, "PeriodZ");
    data["Num"] = in.data.PZ;
  }

}
