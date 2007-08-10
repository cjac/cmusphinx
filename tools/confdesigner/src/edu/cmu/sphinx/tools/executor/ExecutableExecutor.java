package edu.cmu.sphinx.tools.executor;

import edu.cmu.sphinx.util.props.ConfigurationManagerUtils;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ExecutableExecutor implements ActionListener {

    private final PropertySheet executablePS;

    private Thread t;


    public ExecutableExecutor(PropertySheet executablePS) {

        assert ConfigurationManagerUtils.isImplementingInterface(executablePS.getConfigurableClass(), Executable.class);
        this.executablePS = executablePS;
    }


    public void actionPerformed(ActionEvent e) {
        t = new Thread() {

            public void run() {
                super.run();
                try {
                    ((Executable) executablePS.getOwner()).doExecute();
                } catch (PropertyException e1) {
                    e1.printStackTrace();
                }
            }
        };

        t.start();
    }


    public Thread getExcutorThread() {
        return t;
    }
}
