<?php
require_once("/emsincludes/emsqry.inc");

function getWWInfo($src){
  return getInfo($src,"ww requestdata");
}

function getPartyInfo($src){
  return getInfo($src,"hk1 requestdata");
}

function getHKInfo($src){
  return getInfo($src,"hk1 requestdata");
}

function getRCInfo($src){
  return getInfo($src,"rc requestdata");
}

function getUBAInfo($src){
  return getInfo($src,"uba requestdata");
}

function getMaintenanceInfo($src){
  return getInfo($src,"uba requestdata");
}

function getCurrentMaintenanceInfo($src){
  return getInfoCommand($src,"uba checkmaintenanceneeded");
}

function getInfo($src,$command){
  global $cache;
  if (!isset($cache)) $cache=array();
  # Warmwasser
  $srch = array("wwmode" => 	"ww opmode =",
                "zirmode" => 	"zirkpump opmode =",
                "spzir" => 	"zirkpump switchpoints =",
                "wwtag" => 	"ww settemperature =",
                "wwactive" => 	"ww masterswitch =",
                "limittemp" => 	"ww maxtemperature =",
                "loadled" => 	"ww onetimeloadindicator =",
                "tdstat" => 	"ww desinfection =",
                "tdday" => 	"ww desinfectionday =",
                "tdhour" => 	"ww desinfectionhour =",
                "tdtemp" => 	"ww desinfectiontemperature =",
  # Heizkreis
                "day" => 		"hk1 daytemperature =",
                "night" => 		"hk1 nighttemperature =",
                "tagbetr" =>		"hk1 daymode =",
                "autobetr" =>		"hk1 automode =",
                "frost" => 		"hk1 frostsafetemperature =",
                "temptemp" => 		"hk1 temperatureoverride =",
                "nachtred" => 		"hk1 reducedmodethreshold =",
                "urlaubnachtred" => 	"hk1 vacationreducedmodethreshold =",
                "absquit"  =>	 	"hk1 cancelreducedmodethreshold =",
                "urlaubt"  =>	 	"hk1 vacationtemperature =",
                "summertime" => 	"hk1 summerwinterthreshold =",
                "auslegtemp" => 	"hk1 designtemperature =",
                "minvorlauf" => 	"hk1 mintemperature =",
                "maxvorlauf" => 	"hk1 maxtemperature =",
                "redmode" => 		"hk1 controltype =",
                "refinput" => 		"hk1 relevantparameter =",
                "refinputvac" => 	"hk1 vacationreductionmode =",
                "maxroomeffect" => 	"hk1 maxroomeffect =",
                "schedoptimizer" => 	"hk1 scheduleoptimizer =",
                "frostmode" => 		"hk1 frostsafemode =",
                "raumoffset" => 	"hk1 roomoffset =",
  # RC35
                "minaussentemp" => 	"outdoor mintemperature =",
                "gebaeude" => 		"buildingtype =",
                "daempfung" => 		"damping =",
  # UBA
                "antipen" => 		"antipendelminutes =",
                "heizactive" => 	"heater masterswitch =",
                "kpmin" => 		"heaterpump minmodulation =",
                "kpmax"  => 		"heaterpump maxmodulation =",
                "kpnachl"  => 		"heaterpump followupminutes =",
                "burnmin" => 		"burner minmodulation =",
                "burnmax"  => 		"burner maxmodulation =",
                "hystein"  => 		"heater onhysteresis =",
                "hystaus"  => 		"heater offhysteresis =",
                "totalhours"  => 	"operatingminutes =",
  # Party
                "pause" => 		"hk1 pausehours =",
                "party" => 		"hk1 partyhours =",
  # Wartung
                "mtmode" => 		"heater maintenancereminder =",
                "mthours" => 		"heater maintenanceintervalin100hours =",
                "mtdate"  => 		"heater maintenancedate = ",
                "mtactive"  => 		"heater maintenancedue =",
  );

  if (isset($cache[$command])){
    $res = $cache[$command];
  } else {
    $res=doEmsDataCommand($command);
    $cache[$command]=$res;
  }

  $searchstr = $srch[$src];

  foreach($res as $r){
    $r = utf8_decode($r);
    $r2 = str_replace($searchstr,"",$r);        
    if ($r2 != $r) {
      # Richtige Zeile gefunden!
      $ra = explode("|",$r2);
      switch($src){
        case "mtdate":
          $ra[0] = flipdater($ra[0]);
          break;
      }
      return trim($ra[0]);
    }
  }
}

function flipdater($s){
 if ($s=="") $s = "2000-01-01";
  $s = str_replace("-",".",$s);
  $sa = explode(".",$s);
  $day = substr("00".trim($sa[2]),-2);
  $month = substr("00".$sa[1],-2);
  $year = "2".substr("000".$sa[0],-3);
  $res = ($day.".".$month.".".$year);
  return $res;
}

function getHolVacInfo($src){
  $res = doEmsCommand("hk1 get".$src);  
  $out = array();
  foreach($res as $l){
    if (substr($l,0,5)=="begin") $out["von"] = flipdater(substr($l,6,10));
    if (substr($l,0,3)=="end") $out["bis"] = flipdater(substr($l,4,10));
  }
  return $out;  
}

function getContactInfo($src){
  $res = doEmsCommand("rc getcontactinfo ");  
  $out = array();
  foreach($res as $l){
    if ($l!="OK\n") $out[] = trim($l);
  }
  return $out[$src-1];;  
}
?>