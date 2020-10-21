"{\
  'general':{\
    'name':'Hub',\
    'instance':1,\
    'interval':5000},\
  'components':[\
    {'name':'Ethernet','params':['Ether1',[/*Mac address in decimal*/],[192,168,0,1]]},\
    {'name':'GoogleSheets', 'params':['Google', 7001, '/macros/s/<your-script-id>/exec','<your-sheet-id>', false]},\
    {'name':'LoRa','params':[255,3,23,3,200]}\
  ]\
}"
