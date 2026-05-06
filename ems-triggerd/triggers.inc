<?php

function trigger($data,$key,$value,$comp="="){
  if (!isset($data[$key])) return false;
  $cval = $data[$key]["value"];
  switch ($comp){
  case "=":
    if ($cval == $value) return true;
    break;
  case "<":
    if ($cval < $value) return true;
    break;
  case ">":
    if ($cval > $value) return true;
    break;
  }
  return false;
}

$lastpumpforce = 0;
$maxflushes = 0;

function processtriggers($data,$prev){
  global $lastpumpforce, $maxflushes;

  if (trigger($data,"heater pumpactive","off") && trigger($data,"heatexchanger currenttemperature",70,">") && ($lastpumpforce == 0) ){
    # heatexchanger overtemp while pump off. Flush it.
    slog("Heatexchanger quite hot and pump is off! Flush it.");
    var_dump(doEmsCommand("uba testmode on 0 100 0 0"));
    $lastpumpforce = mktime();
    $maxflushes = 30;
  }

  if (($lastpumpforce > 0 )&& (mktime() - $lastpumpforce > 15)){
    var_dump(doEmsCommand("uba testmode on 0 100 0 0"));
    slog("Continue flushing ($maxflushes)... ");
    $lastpumpforce = mktime();
    $maxflushes--;
  } 

  if (($lastpumpforce > 0) && trigger($data,"heatexchanger currenttemperature",50,"<")){
    if ($maxflushes > 10){
      slog("Heatexchanger is fairly cool again. Preparing for flush stop.");
      $maxflushes = 10; # flush stop in 2.5 minutes.
    }
  } 

  if (($lastpumpforce > 0) && ($maxflushes <= 0)){
    var_dump(doEmsCommand("uba testmode off "));    
    slog("Stop flushing. Timeout exceeded.");
    $lastpumpforce = 0;
  } 

}