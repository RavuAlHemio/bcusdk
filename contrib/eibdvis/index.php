<?php
/*
    EIBD client library examples
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
include('common.php');
?>
<html>
  <head>
    <title><?php print htmlentities($config->title);?></title>
  </head>
  <frameset rows = "*, 5%" frameborder="0">
    <frameset cols = "20%, *" frameborder="0">
      <frame src = "list.php" name = "list"/>
      <frame src = "room.php" name = "room" />
    </frameset>
    <frame src = "send.php" name = "send" />
  </frameset>
</html>