#include "eGreenhouseJSON.h"


static bool get_data_point_from_contents(const JsonArrayConst& contents, const char* module_name, const char* data_key, float& out){
    for(const JsonVariantConst& module_data: contents){
        const char* name = module_data["module"];
        if (name == nullptr){
            continue;
        }
        if (strncmp(name, module_name, 15) != 0){
            continue;
        }

        const JsonObjectConst data_obj = module_data["data"];

        if (data_obj.isNull()){
            continue;
        }

        const JsonVariantConst data_value = data_obj[data_key];

        if (!data_value.isNull() && data_value.is<float>()){
            out = data_value.as<float>();
            return true;
        }
    }

    out = NAN;
    return false;
}

static bool get_data_point_from_contents_int(const JsonArrayConst& contents, const char* module_name, const char* data_key, int& out) {
  for (const JsonVariantConst& module_data : contents) {

    const char* name = module_data["module"];
    if (name == nullptr)

      continue;

    if (strncmp(name, module_name, 15) != 0)
      continue;

    const JsonObjectConst data_obj = module_data["data"];

    if (data_obj.isNull())
      continue;

    const JsonVariantConst data_value = data_obj[data_key];
    if (!data_value.isNull() && data_value.is<int>()) {

      out = data_value.as<int>();

      return true;
    }
  }

  out = -333;
  return false;
}


void json_to_struct(const JsonObjectConst& data, eGreenhouse_Base& out) {
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

  // packet #
  get_data_point_from_contents_int(contents, "Packet", "Number", out.data.packageNum);

    // SHT31D temperature
  get_data_point_from_contents(contents, "SHT31D", "temp", out.data.sht31dTemp);
    
    // SHT31D humidity
  get_data_point_from_contents(contents, "SHT31D", "humid", out.data.sht31dHumidity);

    // TSL2591 Vis
  get_data_point_from_contents_int(contents, "TSL2591", "Vis", out.data.tsl2591Vis);

    // TSL2591 IR
  get_data_point_from_contents_int(contents, "TSL2591", "IR", out.data.tsl2591IR);

    // TSL2591 Full
  get_data_point_from_contents_int(contents, "TSL2591", "Full", out.data.tsl2591Full);

    // K30
  get_data_point_from_contents(contents, "K30", "C02", out.data.k30);

    // X_Location
   get_data_point_from_contents_int(contents, "X_Location", "MM", out.data.X);

   // Y_Location
   get_data_point_from_contents_int(contents, "Y_Location", "MM", out.data.Y);

   // X_Location
   get_data_point_from_contents_int(contents, "Z_Location", "MM", out.data.Z);

   // Boolean_Hyper_Move
   get_data_point_from_contents_int(contents, "HyperRail_Passes", "Boolean", out.data.done);

  // timestamp
  const JsonObjectConst stamp = data["timestamp"];
  if (!stamp.isNull()) {
    const char* date = stamp["date"].as<const char*>();
    if (date != nullptr)
      strncpy(out.data.datestamp, date, 15);
    else
      out.data.datestamp[0] = 0;
    const char* time = stamp["time"].as<const char*>();
    if (time != nullptr)
      strncpy(out.data.timestamp, time, 15);
    else
      out.data.timestamp[0] = 0;
  }
}

static JsonObject make_module_object(const JsonArray& contents, const char* name) {
  const JsonObject data = contents.createNestedObject();
  data["module"] = name;
  return data.createNestedObject("data");
}

void struct_to_json(const eGreenhouse_Base& in, const JsonObject& out) {
  // start adding object to it!
  out["type"] = "data";
  // id block
  const JsonObject id = out.createNestedObject("id");
  id["name"] = in.data.name;
  id["instance"] = in.data.instance;
  // timestamp block
  const JsonObject timestamp = out.createNestedObject("timestamp");
  timestamp["date"] = in.data.datestamp;
  timestamp["time"] = in.data.timestamp;
  // contents array
  const JsonArray contents = out.createNestedArray("contents");
  // add objects for each sensor!

  // pkt number
  {
    const JsonObject data = make_module_object(contents, "Packet");
    data["Number"] = in.data.packageNum;
  } 

  // sht31d
  {
    const JsonObject data = make_module_object(contents, "SHT31D");
    data["temp"] = in.data.sht31dTemp;
    data["humid"] = in.data.sht31dHumidity;
    
  }

  //tsl2591
  {
    const JsonObject data = make_module_object(contents, "TSL2591");
    data["Vis"] = in.data.tsl2591Vis;
    data["IR"] = in.data.tsl2591IR;
    data["Full"] = in.data.tsl2591Full;
  }

  //k30
  {
    const JsonObject data = make_module_object(contents, "K30");
    data["CO2"] = in.data.k30;
  }

  //X_Location
  {
    const JsonObject data = make_module_object(contents, "X_Location");
    data["MM"] = in.data.X;
  }
  
  //Y_Location
  {
    const JsonObject data = make_module_object(contents, "Y_Location");
    data["MM"] = in.data.Y;
  }

    //Z_Location
  {
    const JsonObject data = make_module_object(contents, "Z_Location");
    data["MM"] = in.data.Z;
  }

   //Hyper_Rail_Move
   {
    const JsonObject data = make_module_object(contents, "HyperRail_Passes");
    data["Boolean"] = in.data.done;
   }
}
