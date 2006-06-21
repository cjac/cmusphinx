<%--
  Created by IntelliJ IDEA.
  User: bertrand
  Date: Mar 23, 2006
  Time: 1:04:44 PM
  Simple and initial jsp just gathers the user name and the password from the user
  and passes the info off to login_process.jsp
--%>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<%  if (request.getParameter("logout")!=null) session.invalidate(); %>
<html>
<head>
    <title>Text Entry Project - Data Collection Login</title>
</head>

<body>
<center>
    <h1>AudioCollector (Text Entry Project)</h1>

    <h2><%=request.getParameter("msg") == null ? "" : request.getParameter("msg")%></h2>

    <form action="login_process.jsp" method="post">
        <TABLE width="80%" align=center border=3>
            <tr>
                <th><font color="RED">USER</font></th><td><input type="text" name="userID" size="20"></td>
                <th><font color="RED">PASSWORD</font></th><td><input type="password" name="password" size="20"></td>
            </tr>
        </table>
        <input type="submit" value="     login      ">
    </form>
</center>
</body>
</html>