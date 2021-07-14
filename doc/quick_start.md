# 1、新建项目

将本目录下的demo文件夹拷贝至你的web目录下，即可访问

### 目录结构

```
├── app
│   ├── Bootstrap.php
│   ├── conf
│   │   └── config.php   //配置文件
│   ├── controllers
│   │   └── Index.php   //默认控制器
│   ├── models          //models目录
│   │   └── Admin.php
│   ├── routers
│   │   └── router.php  //路由器配置文件
│   └── views           //视图目录
│       └── index
│           └── two.php
├── composer.json
├── composer.lock
├── vendor
│   
└── web
    └── index.php  //入口文件

```

### 入口文件
入口文件是所有请求的入口, 一般都借助于rewrite规则, 把所有的请求都重定向到这个入口文件.

```
<?php
$appPath = __DIR__ . "/../app";  //指向app目录的路径
$app = new Lxx\Application($appPath);

$app->run();

```


### 控制器
lxx中没有默认的控制器，一切都是由路由决定。控制器class名字必须与该控制器文件一致，否则会导致找不到控制器文件。
```
<?php
namespace controllers;
use Lxx\Controller;

class Index extends Controller {

    public function Index() {
        $str = "Hello Lxx framework";
        $this->Text($str );
    }
}
```

### 路由
lxx框架的路由是采用rax基数树算法实现。
```
<?php

$router = Lxx\Application::app()->getRouter();

$router->get("/", "Index@Index");
$router->get("/one/{name}", "Index@One");
$router->get("/two", "Index@Two");
```

### 运行
在浏览器输入http://localhost就能看到输出Hello Lxx framework了。