#!/usr/bin/php 
<?php
slog("EMS-Triggerd started. Waiting a little bit.");
sleep(15);
slog("EMS-Triggerd resuming operations.");
$ems_address = '127.0.0.1';
$ems_service_port = 7778;

require("/emsincludes/emsqry.inc");
require("triggers.inc");

function slog($text){
  print(date("d.m.Y H:i:s").' '.$text."\n");
}
  

error_reporting(0);
# Open Socket to EMS-Info-Server

$ems_data_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

if ($ems_data_socket === false) {
    die ("Creation of socket to collectord failed: " . socket_strerror(socket_last_error()) . "\n");
  }

$result = socket_connect($ems_data_socket, $ems_address, $ems_service_port);
if ($result === false) {
  die ("Connect to collectord failed ($result): " . socket_strerror(socket_last_error($ems_data_socket)) . "\n");
}
$data = array();
$data_prev = array();
              
while (1){
  while ($in = socket_read($ems_data_socket, 2048)) {
    $tstamp = mktime();
    $data_prev = $data;
    $ina = explode("\n",$in);
    foreach ($ina as $l){
      $la = explode(" ",trim($l));
      $key = trim(array_shift($la));
      if (count($la)>1) $key .= " ".trim(array_shift($la));
      $value = trim(implode(" ",$la));       
      if (strtok($key," ") == "systemtime"){
        $value = strtok(" ")." ".$value;
        $key = "systemtime";
      }
      if ((trim($key)!="") && (trim($value)!="")) $data[$key] = array("value"=>$value, "time"=>$tstamp);
    }
    ksort($data);
    processtriggers($data, $data_prev);
  }
  socket_shutdown($ems_data_socket);
  socket_close($ems_data_socket);
  # Read failed, probably the collectord died.  Wait and try reconnecting.
  sleep(2);
  $ems_data_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

  if ($ems_data_socket === false) {
    die ("Creation of socket to collectord failed: " . socket_strerror(socket_last_error()) . "\n");
  }
  $result = socket_connect($ems_data_socket, $ems_address, $ems_service_port);
  if ($result === false) {
    die ("Connect to collectord failed ($result): " . socket_strerror(socket_last_error($ems_data_socket)) . "\n");
  }
}
socket_close ($ems_data_socket);
?>