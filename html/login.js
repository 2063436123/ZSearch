
function showMessage() {
  var username = document.getElementById("username").value;
  var password = document.getElementById("password").value;
  console.log(username, password);

  if (window.ActiveXObject) {
    //如果是IE浏览器
    xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
  } else if (window.XMLHttpRequest) {
    //非IE浏览器
    xmlHttp = new XMLHttpRequest();
  }

  xmlHttp.open("POST", "http://114.116.103.123:9090/login", false); //true表示异步请求，false表示同步
  xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  //2.在回调函数中对服务器响应的数据进行处理
  xmlHttp.onreadystatechange = function () {
    console.log("status" + xmlHttp.status);
    //判断status响应状态码是否为200，readyState 就绪码是否是4
    if (xmlHttp.status == 200 && xmlHttp.readyState == 4) {
      //获取服务器返回的内容,响应的结果
      //获取服务器返回的内容
      var responseText = xmlHttp.responseText;
      window.location.href = responseText;
    } else {
      alert("账号或密码错误!");
    }
  };

  //3.使用open方法建立与服务器的链接
  xmlHttp.send("username=" + username + "&password=" + password);
}
