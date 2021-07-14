<?php
namespace controllers;
use Lxx\Controller;

class Index extends Controller {

    public function Index() {
        $str = "Hello Lxx framework";
        $this->Text($str );
    }

    public function One() {
        $value = $this->Request()->getParam("name");
        $this->Json(['name'=>$value]);
    }

    public function Two() {
        
        $this->view("index/two", ["name"=> "Two"]);
    }
}