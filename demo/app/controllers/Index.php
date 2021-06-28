<?php
namespace controllers;
use Lxx\Controller;

class Index extends Controller {

    public function Index() {
        $str = "Controller is ".$this->router()->controller().", Action is ".$this->router()->action();
        
        return $str;
    }
}