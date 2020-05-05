"{\
  'general':{'name':'Device', 'instance':1,'interval':5000},\
  'components':[\
    {'name':'TSL2591','params':'default'},\
    {'name':'SHT31D','params':'default'},\
    {'name':'K30','params':'default'},\
    {'name':'DS3231','params':'default'},\
    {'name':'Sleep_Manager','params':[true,false,1]},\
    {'name':'Interrupt_Manager','params':[0]},\
    {'name':'SD','params':[true,1000,10,'data',true]},\
    {'name':'LoRa','params':[255,3,23,3,200]}\
  ]\
}"