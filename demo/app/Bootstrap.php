<?php

class Bootstrap {

    public function init() {
        \ActiveRecord::setDb(new PDO('mysql:host=localhost;dbname=blog', "root", ""));
    }
}