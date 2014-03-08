#!/usr/bin/php
<?php
$basedir= "/home/programming/Heizung/ems-collector/collector/";
$typef = file($basedir."EmsMessage.h");
$dhf = file($basedir."DataHandler.cpp");
$iof = file($basedir."IoHandler.cpp");

function extr ($in,$srch){
  $act = false;
  $res = array();
  foreach ($in as $l){
    if (trim($l) == $srch) {$act = true;continue;};
    if (trim($l) == "};") $act = false;
    if ($act){
    $l = trim($l);
    if (substr($l,0,2)=="/*") continue;
    $p = strpos($l,"/*");
    if ($p) $l = trim(substr($l,0,$p));
    $l = str_replace(",","",$l);
    if ($l) $res[]=trim($l);
    }
  }
  return $res;
}  

function sstrip ($in){
  $res = array();
  foreach($in as $l){
    $l = str_replace("{ EmsValue::","",$l);
    $l = trim(str_replace("}","",$l));
    $la = explode('"',$l);
    $res[trim($la[0])]=trim($la[1]);
  }
  return $res;
}  

$ty=(extr($typef,"enum Type {"));
$sty=(extr($typef,"enum SubType {"));
$tmd=(sstrip(extr($dhf,"static const std::map<EmsValue::Type, const char *> TYPEMAPPING = {")));
$stmd=(sstrip(extr($dhf,"static const std::map<EmsValue::SubType, const char *> SUBTYPEMAPPING = {")));

print("\nTypes DH: ");
foreach ($ty as $t){
  if ($t == "None") continue;
  print(".");
  if (!array_key_exists($t,$tmd)) print("No mapping for $t!\n");
}
print("\nSubTypes DH: ");
foreach ($sty as $t){
  if ($t == "None") continue;
  print(".");
  if (!array_key_exists($t,$stmd)) print("No mapping for $t!\n");
}

$tmi=(sstrip(extr($iof,"static const std::map<EmsValue::Type, const char *> TYPEMAPPING = {")));
$stmi=(sstrip(extr($iof,"static const std::map<EmsValue::SubType, const char *> SUBTYPEMAPPING = {")));
$units = (sstrip(extr($iof,"static const std::map<EmsValue::Type, const char *> UNITMAPPING = {")));
print("\nTypes IO: ");
foreach ($ty as $t){
  if ($t == "None") continue;
  print(".");
  if (!array_key_exists($t,$tmi)) print("No mapping for $t!\n");
}


print("\nSubTypes IO: ");
foreach ($sty as $t){
  if ($t == "None") continue;
  print(".");
  if (!array_key_exists($t,$stmi)) print("No mapping for $t!\n");
}
print("\n");


$res = array();
$ures = array();

foreach ($tmd as $k => $v){
  $res[$v] = utf8_decode($tmi[$k]);
  $ures[$v] = "";
  if (isset($units[$k])) $ures[$v] = utf8_decode($units[$k]);
}

foreach ($stmd as $k => $v){
  if (isset($res[$v])) die ("Conflict: $v\n");
  $res[$v] = utf8_decode($stmi[$k]);
}
print("\n\n");
print('$m_text=');
var_export($res);
print("\n".'$m_units=');
var_export($ures);
print("\n");
