"{\
  'general':{\
    'name':'eGH_Package',\
    'instance':1,\
    'interval':5000\
    },\
  'components':[\
    {'name':'TSL2591','params':'default'},\
    {'name':'SHT31D','params':'default'},\
    {'name':'K30','params':'default'},\
    {'name':'DS3231','params':[10,true]},\
    {'name':'SD','params':[true,1000,10,'eGreen',true]},\
    {'name':'LoRa','params':[255,9,23,3,200]}\
  ]\
}"
  
