#include "eGreenhouseJSON.h"

static JsonObject hub_make_module_object(const JsonArray& contents, const char* name) {
  const JsonObject data = contents.createNestedObject();
  data["module"] = name;
  return data.createNestedObject("data");
}

void hub_struct_to_json(const eGreenhouse_Base& in, const JsonObject& out) {
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
    const JsonObject data = hub_make_module_object(contents, "Packet");
    data["Number"] = in.data.packageNum;
  } 

  // sht31d
  {
    const JsonObject data = hub_make_module_object(contents, "SHT31D");
    data["temp"] = in.data.sht31dTemp;
    data["humid"] = in.data.sht31dHumidity;
    
  }

  //tsl2591
  {
    const JsonObject data = hub_make_module_object(contents, "TSL2591");
    data["Vis"] = in.data.tsl2591Vis;
    data["IR"] = in.data.tsl2591IR;
    data["Full"] = in.data.tsl2591Full;
  }

  //k30
  {
    const JsonObject data = hub_make_module_object(contents, "K30");
    data["CO2"] = in.data.k30;
  }

  //X_Location
  {
    const JsonObject data = hub_make_module_object(contents, "X_Location");
    data["MM"] = in.data.X;
  }
  
  //Y_Location
  {
    const JsonObject data = hub_make_module_object(contents, "Y_Location");
    data["MM"] = in.data.Y;
  }

    //Z_Location
  {
    const JsonObject data = hub_make_module_object(contents, "Z_Location");
    data["MM"] = in.data.Z;
  }

   //Hyper_Rail_Move
   {
    const JsonObject data = hub_make_module_object(contents, "HyperRail_Passes");
    data["B"] = in.data.done;
   }
}
