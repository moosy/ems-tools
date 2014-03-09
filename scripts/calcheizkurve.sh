#!/usr/bin/php
<?php
$incdir = "/emsincludes/";
require($incdir."emsgetinfo.inc");

$AT = getHKInfo("auslegtemp");
$RTO = getHKInfo("raumoffset");
$TT = getHKInfo("day");
$TN = getHKInfo("night");
$MAUT = getRCInfo("minaussentemp")+0;

$data = getEmsLiveData();

$GAP = $data["outdoor dampedtemperature"];


$ATT= $AT + $RTO + $TT - 19;
$BTT= $RTO + $TT + 1;

$ATN= $AT + $RTO + $TN - 21;
$BTN= $RTO + $TN - 1;

$f1 = fopen($tmpdir."/hkt.dat","w");
fwrite($f1,"$MAUT $ATT\n");
fwrite($f1,"20  $BTT\n");
fclose($f1);

$f1 = fopen($tmpdir."/hkn.dat","w");
fwrite($f1,"$MAUT $ATN\n");
fwrite($f1,"20  $BTN\n");
fclose($f1);


$f1 = fopen($tmpdir."/hka.dat","w");
fwrite($f1,"$GAP 5\n");
fwrite($f1,"$GAP 45\n");
fclose($f1);


if (getHKInfo("redmode")=="offmode") unlink($tmpdir."/hkn.dat");

$f1 = fopen($tmpdir."/temp.dat","w");
fwrite($f1,"$AT\n$RTO\n$TT\n$TN\n$MAUT\n");
fclose($f1);

exec($emsscriptpath."/ems-heizkurve.py");

 