<?php
namespace controllers;
use Lxx\Controller;

class Index extends Controller {

    public function Index() {
        $str = "Controller is ".$this->router()->controller().", Action is ".$this->router()->action();
        $request = $this->request();
        var_dump($request);
        return $str;
    }
}