package edu.cmu.sphinx.tools.confdesigner.util;

import edu.cmu.sphinx.tools.confdesigner.ClassPathParser;
import edu.cmu.sphinx.tools.confdesigner.ConfDesigner;
import edu.cmu.sphinx.util.props.PropertyException;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class JNLPDemoStarter {

    public static void main(String[] args) throws IOException, PropertyException {
        ConfDesigner gui = new ConfDesigner();

        assert System.getProperty("jnlpx.jvm") != null;

        List<String> ignoreList = Arrays.asList("jsapi.jar", "org-netbeans-modules-visual-examples.jar", "l2fprod-common-sheet.jar", "org-openide-util.jar", "org-netbeans-api-visual.jar", "xstream-1.2.jar");
        gui.addConfigurables(ClassPathParser.getConfigurablesWhileWebstarting(ignoreList));

        gui.setBounds(200, 100, 900, 700);
        gui.setVisible(true);
    }
}
