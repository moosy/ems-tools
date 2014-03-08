<?php
require("/emsincludes/config.php");
$daystr = array("0" => "Montag",
		"1" => "Dienstag",
		"2" => "Mittwoch",
		"3" => "Donnerstag",
		"4" => "Freitag",
		"5" => "Samstag",
		"6" => "Sonntag");
$daystr2 = array("monday" => "Montag",
		 "tuesday" => "Dienstag",
		 "wednesday" => "Mittwoch",
	 	 "thursday" => "Donnerstag",
		 "friday" => "Freitag",
		 "saturday" => "Samstag",
		 "sunday" => "Sonntag");

function flush_buffers(){ 
     ob_end_flush(); 
     ob_flush(); 
     flush(); 
     ob_start(); 
}

function open_ems(){
  global $ems_socket;
  $address = "127.0.0.1";
  $service_port=7777;
  
  if (!isset($ems_socket)){
    $ems_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

    if ($ems_socket === false) {
      die ("FATAL: Keine Verbindung zum EMS-Bus!\nGrund: " . socket_strerror(socket_last_error($ems_socket)) . "\n");
    }

    $result = socket_connect($ems_socket, $address, $service_port);
    if ($result === false) {
      die ("FATAL: Keine Verbindung zum EMS-Bus!\nGrund: " . socket_strerror(socket_last_error($ems_socket)) . "\n");
    }
    socket_set_option($ems_socket, SOL_SOCKET, SO_RCVTIMEO, array('sec'=>0, 'usec'=>5000));
  }
  return true;
}

function open_ems_data(){
  global $ems_data_socket;
  $address = "127.0.0.1";
  $service_port_data=7779;
  
  if (!isset($ems__data_socket)){
    $ems_data_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

    if ($ems_data_socket === false) {
      die ("FATAL: Keine Verbindung zum EMS-Cache!\nGrund: " . socket_strerror(socket_last_error($ems_data_socket)) . "\n");
    }

    $result = socket_connect($ems_data_socket, $address, $service_port_data);
    if ($result === false) {
      die ("FATAL: Keine Verbindung zum EMS-Cache!\nGrund: " . socket_strerror(socket_last_error($ems_data_socket)) . "\n");
    }
    socket_set_option($ems_data_socket, SOL_SOCKET, SO_RCVTIMEO, array('sec'=>0, 'usec'=>1000));

  }
  return true;
}

function close_ems(){
  global $ems_socket, $ems_data_socket;
  $tmp = "quit\n";
  if (isset($ems_socket)){
    socket_shutdown($ems_socket,1);
    while (socket_read($ems_socket, 1)); # Remove garbage
    socket_close($ems_socket);
  }
  if (isset($ems_data_socket)){
    socket_write($ems_data_socket, $tmp, strlen($tmp));
    socket_shutdown($ems_data_socket,1);  
    while (socket_read($ems_data_socket, 1)); # Remove garbage
    socket_close($ems_data_socket);
  }  
}


function idiv($a,$b){
  return (($a - ($a % $b)) / $b);
}

function doEmsCommand($in){
  global $ems_socket;
  open_ems();
  $in = $in."\n";
  do {
    while (socket_read($ems_socket, 32)); # Remove garbage
    socket_write($ems_socket, $in, strlen($in));
    $res = "";
    $tout = 0;
    while (1) {
          $out = socket_read($ems_socket, 32);
          $tout++;
          if (strlen($out)>0) $tout=0;
          if ($tout > 1000) {  // Timeout 5s (5ms * 1000)
            break;
          }
          $res .=$out;
          if (strpos(" ".$res,"ERRBUSY")) {
            usleep(500000);
            break;
          }

          if (strpos(" ".$res,"OK"))  break 2;
          if (strpos(" ".$res,"ERRARGS")) break 2;
          if (strpos(" ".$res,"ERRCMD")) break 2;
    }
  } while (strpos(" ".$res,"ERRBUSY"));

  $resa = explode("\n",$res);
  if (!trim(end($resa))) array_pop($resa);
  return $resa;

}

function doEmsDataCommand($in="",$wait=false){
  global $ems_socket, $ems_data_socket, $reqdone;
  if (!isset($reqdone)) $reqdone = array();
  $in = trim($in);
  $checks=array("hk1 requestdata" => "hk1 partyhours",
		"hk2 requestdata" => "hk2 partyhours",
		"hk3 requestdata" => "hk3 partyhours",
		"hk4 requestdata" => "hk4 partyhours",
		"ww requestdata" => "ww desinfectionday",
                "rc requestdata" => "buildingtype",
		"uba requestdata" => "heaterpump followupminutes");
  $check = "OK";
  $oldest = 0;
  if ($in){
    $oldest = mktime();

    if ($wait || !isset($reqdone[$in])){  # If wait is forced or data not yet available get it
      if (isset($checks[$in])) $check = $checks[$in];
      doEmsCommand($in); 
      $reqdone[$in] = true;
    } 
  }
  $tout2=0;
  open_ems_data();
  while ($tout2 < 20){
    $tout2++;
    $tmp = "getdata\n";
    socket_write($ems_data_socket, $tmp, strlen($tmp));
    $res = "";
    $tout = 0;
    while ($out = socket_read($ems_data_socket, 1024)) {
      if (strlen($out)==0) $tout++;
      if ($tout > 20) {
        break;
      }
      $res .=$out;
      if (strpos(" ".$res,"OK")) { break;}
    }

    $rp = strpos(" \n".$res,$check);
    if ($rp){
      if ($check == "OK") break;
      $val = substr(" \n".$res,$rp);
      $ep = strpos($val,"\n");
      $val = substr($val,0,$rp);
      $vala = explode("|",$val);
      $valtime = trim($vala[1]);

      if ($valtime > $oldest){
        break;
      }
    }

    usleep(100000);

  }
  $resa = explode("\n",$res);
  if (!trim(end($resa))) array_pop($resa);
  return $resa;
}


function parseTime($in){
  //  Convert MO 12:34 in minutes since Monday, 0:00

  $si = explode(" ",$in);
  $day=99;
  if ($si[0] == "monday") $day=0;
  if ($si[0] == "tuesday") $day=1;
  if ($si[0] == "wednesday") $day=2;
  if ($si[0] == "thursday") $day=3;
  if ($si[0] == "friday") $day=4;
  if ($si[0] == "saturday") $day=5;
  if ($si[0] == "sunday") $day=6;
  $sj = explode(":",$si[1]);
  if ($day==99) return false;
  return ($day*24*60 + $sj[0]*60 + $sj[1] - ($sj[1] % 10) ); // 10 Minuten Raster!
}

function day2str($day){
  $days = "monday";
  if ($day == 0) $days="monday";
  if ($day == 1) $days="tuesday";
  if ($day == 2) $days="wednesday";
  if ($day == 3) $days="thursday";
  if ($day == 4) $days="friday";
  if ($day == 5) $days="saturday";
  if ($day == 6) $days="sunday";
  return $days;

}

function synthTime($in){
  // converts minutes since Mon 00:00 to MO 12:34
  $day = idiv($in,24*60);
  return (day2str($day)." ".hm2str( ($in % (24*60))*60 ));
  
}

function time2dayhm($in){
  // converts minutes since Mon 00:00 to 0 12:34
  $day = idiv($in,24*60);
  
  return (array($day,($in % (24*60))*60));
  
}

function hm2str($in){
  if ($in === false) return "";
  return date("H:i",$in-60*60);
}


function parseSchedule($in){
#  var_dump($in);
  $sch = array();
  foreach ($in as $l){
    if (trim($l) == "OK") continue;
    $si = explode(" ",$l);
    if (count($si)<3) continue;
    $tm = parseTime($si[1]." ".$si[2]);
    $cmd = (trim($si[3])=="on"?true:false);
    if ($tm !== false) $sch[$tm]=$cmd;
  }
  ksort($sch);
  $sch2 = array();
  $old = "INVALID";
  foreach($sch as $k => $v){
    if ($old !== $v){
      $sch2[$k] = $v;
      $old=$v;
    }
  }

  return $sch2;
}

function printSchedule($sch){
  foreach ($sch as $s => $cmd){
    print(synthTime($s).
      " ".($cmd?'on':'off')."\n");
  }
}

function schedule2graph($sch,$outfn){
  global $daystr;
  global $graphpath;
#  var_dump($sch);

  $wechsel = array();
  foreach($sch as $in => $v){
    $i = time2dayhm($in);
    $day = $i[0];
    $wechsel[$day] = $v;
  }

  $doffset = mktime(0,0,0,1,3,2000); # Align Monday
  $fd=array();
  $fname=array();

  $wechsel = array();
  foreach($sch as $in => $v){
    $i = time2dayhm($in);
    $day = $i[0];
    $wechsel[$day+1] = $v;
  }

  for ($k=1;$k<=7;$k++) if (isset($wechsel[$k])) $wechsel[0]=$wechsel[$k];

  for ($k=1;$k<=7;$k++) if (!isset($wechsel[$k])) $wechsel[$k]=$wechsel[$k-1];

  for ($day=0;$day<=6;$day++){
    $fname[$day] = "/tmp/schedule-".time()."-".$day.".dat";
    $fd[$day] = fopen($fname[$day],"w");
    $v = $wechsel[$day];
    fwrite($fd[$day],"2000-01-03 00:00:00 ".($v?(6-$day)+0.3:(6-$day))."\n");
  }
  foreach($sch as $in => $v){
    $i = time2dayhm($in);
    $day = $i[0];
    $hm  = $i[1];
    fwrite($fd[$day],date("Y-m-d H:i:s",$hm + $doffset)." ".($v?(6-$day):(6-$day)+0.3)."\n");
    fwrite($fd[$day],date("Y-m-d H:i:s",$hm + $doffset)." ".($v?(6-$day)+0.3:(6-$day))."\n");
  }  

  for ($day=0;$day<=6;$day++){
    $v = $wechsel[$day+1];
    fwrite($fd[$day],"2000-01-03 23:59:30 ".($v?(6-$day)+0.3:(6-$day))."\n");
    fclose($fd[$day]);
  }


    $p = popen("gnuplot", "w");
    fwrite($p,"set terminal png font 'arial' 12 size 800, 450\n");
     fwrite($p,"set grid lc rgb '#aaaaaa' lt 1 lw 0,5\n");
     fwrite($p,"set title 'Schaltzeiten'\n");
     fwrite($p,"set xdata time\n");
     fwrite($p,"set xlabel 'Zeit'\n");
     fwrite($p,"set ylabel 'Reduziert / Normal'\n");
     fwrite($p,"set timefmt '%Y-%m-%d %H:%M:%S'\n");
     fwrite($p,"set format x '%H:%M'\n");
     fwrite($p,"set xtics 3600 rotate by -45\n");
     fwrite($p,"set ytics autofreq\n");
     fwrite($p,"set key outside\n");

     fwrite($p,"set output '$graphpath$outfn'\n");
     fwrite($p,"plot");
     $first=true;
     foreach ($fname as $fn){
       if (!$first) fwrite($p," , ");
       $first = false;
       $days = $daystr[substr($fn,-5,1)];
       fwrite($p," '".$fn."' using 1:3 with lines lw 6 title '".$days."' ");
     }
     fwrite($p,"\n");

     pclose($p);

     foreach ($fname as $fn) unlink($fn);

}

function formatval($sinf,$day,$on,$i,$onpadding){
  if ($onpadding){
    if ($i==0) return "";
    $i--;
  }
  $res=hm2str($sinf[$day][$on][$i]);
  return $res;
}

function schedule2html($sch,$id){
  global $daystr,$html_cols;
  $sinf = array();
  foreach($sch as $k => $v){
    $dhm = time2dayhm($k);
    $day = $dhm[0];
    $hm = $dhm[1];
    if (!isset($sinf[$day])) $sinf[$day] = array(0 => array(), 1 => array());
    $sinf[$day][$v][] = $hm;
  }

  for($day=0;$day<=6;$day++){
    $onpadding = (isset($sinf[$day][0][0]) && isset($sinf[$day][1][0]) && ($sinf[$day][0][0] < $sinf[$day][1][0]));
    # Padding mit false;
    for($k=0;$k<$html_cols;$k++){
      $sinf[$day][0][]=false;
      $sinf[$day][1][]=false;
    }
    print("<tr>");
    print("<td bgcolor=#cccccc>".$daystr[$day]."</td>");
    for ($i=0;$i<$html_cols;$i++){
      print("<td bgcolor=#cceecc><input type=text size=5 name=".$id."-".$day."-".$i."-1 value=".formatval($sinf,$day,1,$i,$onpadding)."></td>");
      print("<td bgcolor=#eecccc><input type=text size=5 name=".$id."-".$day."-".$i."-0 value=".formatval($sinf,$day,0,$i,false)."></td>");
    }
    print("</tr>\n");
  }
}


function html2sched($post,$id){
  global $html_cols;
  $schstr = "";
  $idx = 1;
  for($day=0;$day<=6;$day++){
    for ($i=0;$i<$html_cols;$i++){
      if (isset($post[$id."-".$day."-".$i."-1"]) && ($post[$id."-".$day."-".$i."-1"] != "")) {
        $schstr .= $idx." ".day2str($day)." ".$post[$id."-".$day."-".$i."-1"]." on\n";
        $idx++;
      }
      if (isset($post[$id."-".$day."-".$i."-0"])&& ($post[$id."-".$day."-".$i."-0"] != "")) {
        $schstr .= $idx." ".day2str($day)." ".$post[$id."-".$day."-".$i."-0"]." off\n";
        $idx++;
      }
    }
  }
  $sch= parseSchedule(explode("\n",$schstr));
  return $sch;
}

function readSchedule($hk,$sub){
  if ($hk == "wwzirkpump") $hk="ww zirkpump";
  return parseSchedule(doEmsCommand($hk." getcustomschedule ".$sub));
}

function programSchedule($sch,$hk,$sub){
  if ($hk == "wwzirkpump") $hk="ww zirkpump";
  $i = 1;
  foreach ($sch as $s => $cmd){

    $eqry = $hk." customschedule ".$sub." $i ".synthTime($s)." ".($cmd?'on':'off');
    $eres = doEmsCommand($eqry);
    $eres = end($eres);
    if (trim($eres) != "OK") die("Error while programming!");
    $i++;
    print('<script type="text/javascript">progress('.($i*2).');</script>'."\n");
    flush_buffers();
  }
  for ($j=$i ; $j <= 42; $j++){
    $dcmd = doEmsCommand($hk." customschedule ".$sub." $j unset");
    if (trim(end($dcmd)) != "OK") die("Error while erasing!");
    print('<script type="text/javascript">progress('.($j*2).');</script>'."\n");
    flush_buffers();
  }
  print("\n");
}

function getActSchedule($src){
  $srch = array("ww" => "ww customschedule",
                "zir" => "zirkpump customschedule");

  $trans = array("off" => "hk",
                 "on" => "custom");

  if (($src == "ww") || ($src == "zir")){
    doEmsDataCommand("ww requestdata");
    $res = getEmsLiveData();
    return $trans[$res[$srch[$src]]];
  }

  $res = doEmsCommand($src." getactiveschedule");
  array_pop($res);
  return (trim(array_pop($res)));
}

function getEmsStatusCodes(){
  global $mysql_host, $mysql_user, $mysql_pass;
  $link=mysql_connect($mysql_host, $mysql_user, $mysql_pass)
     or die("Keine Verbindung zur Datenbank möglich: " . mysql_error());
  mysql_query("use ems_data");
     
  $result = mysql_query("SELECT a.value as sc, b.value as fc, a.starttime as time ".
                      "FROM state_data a, state_data b ".
                      "WHERE a.sensor = 200 and b.sensor = 201 ".
                      "       and a.starttime = b.starttime ".
                      "ORDER BY time DESC" )
        or die("Abfrage fehlgeschlagen! ". mysql_error());
                 
  $res = array();
  while($row = mysql_fetch_assoc($result) ){
    $res[]=str_replace(" ","|",trim($row['time']))."|".$row['sc']."|".$row['fc'];
  }                                                   
  return $res;
}                                                   

function ldat_parse($s){

  if (substr($s,-4) == " min"){
    $h = $s;
    $s = str_replace(" min","",$s);
    $s2 = ($s % (60));
    $s = $s - $s2;
    $s = $s / (60);
    $s = $s."h ".$s2."min";
    }
  return $s;

}

function getEmsLiveData($withtimes=false){
  $in = doEmsDataCommand();
  $inf = array();
  foreach ($in as $z){
    $p = strpos($z,"=");
    $zi = array(trim(substr($z,0,$p)),substr($z,$p+2));
    $k = trim($zi[0]);
    $v = trim($zi[1]);
    if (!$withtimes){
      $va = explode("|",$v);
      $v = trim($va[0]);
    }

    if ($k && (strpos(" ".$z,"error ")==FALSE)) $inf[$k] = $v;
  }

  ksort($inf);
  return $inf;
}

function diff2col($diff,$limit,$low,$med,$high){
  if ($diff < $limit/2){
    return d2c($diff, $limit/2, $low, $med);
  } else {
    return d2c($diff-$limit/2, $limit/2, $med, $high);  
  }
}

function d2c($diff,$limit,$low,$high){
  $rs = "#";
  for ($i=0 ; $i<3; $i++){
    if ($diff > $limit ) { 
      $res = $high[$i];
    } else {
      $cdiff = ($high[$i]-$low[$i]);
      $res = $low[$i] + ($diff/$limit) * $cdiff;
    }
    $rs.=substr("00".dechex($res),-2);
  }
  return $rs;
}
