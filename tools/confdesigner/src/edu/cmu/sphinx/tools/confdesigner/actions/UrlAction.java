package edu.cmu.sphinx.tools.confdesigner.actions;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.io.IOException;

/**
 * Opens a user-predefined url within the default system browser.
 *
 * @author Holger Brandl
 */
public class UrlAction extends AbstractAction {

    private String url;
    private Frame parent;


    public UrlAction(String name, String url, Frame parent) {
        this.url = url;
        this.parent = parent;

        putValue(NAME, name);
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_F, KeyEvent.ALT_MASK));
    }


    public void actionPerformed(ActionEvent e) {
        showInBrowser(url, parent);
    }


    static boolean showInBrowser(String url, Frame frame) {
        //minimizes the app
        if (frame != null)
            frame.setExtendedState(JFrame.ICONIFIED);

        String os = System.getProperty("os.name").toLowerCase();
        Runtime rt = Runtime.getRuntime();
        try {
            if (os.indexOf("win") >= 0) {
                String[] cmd = new String[4];
                cmd[0] = "cmd.exe";
                cmd[1] = "/C";
                cmd[2] = "start";
                cmd[3] = url;
                rt.exec(cmd);
            } else if (os.indexOf("mac") >= 0) {
                rt.exec("open " + url);
            } else {
                //prioritized 'guess' of users' preference
                String[] browsers = {"epiphany", "firefox", "mozilla", "konqueror",
                        "netscape", "opera", "links", "lynx"};

                StringBuffer cmd = new StringBuffer();
                for (int i = 0; i < browsers.length; i++)
                    cmd.append((i == 0 ? "" : " || ") + browsers[i] + " \"" + url + "\" ");

                rt.exec(new String[]{"sh", "-c", cmd.toString()});
                //rt.exec("firefox http://www.google.com");
                //System.out.println(cmd.toString());

            }
        }
        catch (IOException e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(frame,
                    "\n\n The system failed to invoke your default web browser while attempting to access: \n\n " + url + "\n\n",
                    "Browser Error",
                    JOptionPane.WARNING_MESSAGE);

            return false;
        }
        return true;
    }

}
