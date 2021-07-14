<?php
namespace controllers;
use Lxx\Controller;

class Index extends Controller {

    public function Index() {
        $str = "Hello Lxx framework";
        $this->Text($str );
    }
}