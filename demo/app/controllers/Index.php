<?php
namespace controllers;
use Lxx\Controller;

class Index extends Controller {

    public function Index() {
        $str = "Controller is ".$this->getRouter()->controller().", Action is ".$this->getRouter()->action();
        
        $this->Text($str);
    }

    public function One() {

        $this->Json(["key"=>"value"]);
    }

    public function Two() {
        $this->view("index/two", ["name"=> "Two"]);
    }
}