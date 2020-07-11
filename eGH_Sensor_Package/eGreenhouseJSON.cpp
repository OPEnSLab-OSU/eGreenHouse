#include "eGreenhouseJSON.h"


static bool eGH_get_data_point_from_contents(const JsonArrayConst& contents, const char* module_name, const char* data_key, float& out){
    for(const JsonVariantConst& module_data: contents){
        const char* name = module_data["module"];
        if (name == nullptr){
          out = 0.0;
            continue;
        }
        if (strncmp(name, module_name, 15) != 0){
          out = 5.0;
            continue;
        }

        const JsonObjectConst data_obj = module_data["data"];

        if (data_obj.isNull()){
          out = 10.0;
            continue;
        }

        const JsonVariantConst data_value = data_obj[data_key];

        if (!data_value.isNull() && data_value.is<float>()){
            out = data_value.as<float>();
            return true;
        }
    }
    return false;
}

static bool eGH_get_data_point_from_contents_int(const JsonArrayConst& contents, const char* module_name, const char* data_key, int& out) {
  for (const JsonVariantConst& module_data : contents) {

    const char* name = module_data["module"];
    if (name == nullptr){
      out = 0;
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


void eGH_json_to_struct(const JsonObjectConst& data, eGreenhouse_Base& out) {
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
  eGH_get_data_point_from_contents_int(contents, "Packet", "Number", out.data.packageNum);

    // SHT31D temperature
  eGH_get_data_point_from_contents(contents, "SHT31D", "temp", out.data.sht31dTemp);
    
    // SHT31D humidity
  eGH_get_data_point_from_contents(contents, "SHT31D", "humid", out.data.sht31dHumidity);

    // TSL2591 Vis
  eGH_get_data_point_from_contents_int(contents, "TSL2591", "Vis", out.data.tsl2591Vis);

    // TSL2591 IR
  eGH_get_data_point_from_contents_int(contents, "TSL2591", "IR", out.data.tsl2591IR);

    // TSL2591 Full
  eGH_get_data_point_from_contents_int(contents, "TSL2591", "Full", out.data.tsl2591Full);

    // K30
  eGH_get_data_point_from_contents(contents, "K30", "C02", out.data.k30);

    // X_Location
  eGH_get_data_point_from_contents_int(contents, "X_Location", "MM", out.data.X);

   // Y_Location
  eGH_get_data_point_from_contents_int(contents, "Y_Location", "MM", out.data.Y);

   // X_Location
  eGH_get_data_point_from_contents_int(contents, "Z_Location", "MM", out.data.Z);

   // Boolean_Hyper_Move
  eGH_get_data_point_from_contents_int(contents, "HyperRail_Passes", "B", out.data.done);

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
