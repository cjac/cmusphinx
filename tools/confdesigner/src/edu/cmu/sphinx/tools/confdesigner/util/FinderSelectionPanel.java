/*
 * Created by JFormDesigner on Wed Jun 13 23:41:26 CEST 2007
 */

package edu.cmu.sphinx.tools.confdesigner.util;

import edu.cmu.sphinx.tools.confdesigner.ConfNode;

import javax.swing.*;
import javax.swing.border.MatteBorder;
import java.awt.*;
import java.awt.event.*;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/** @author Holger Brandl */
public class FinderSelectionPanel extends JDialog {

    private ConfNode selectedNode;

    Map<String, ConfNode> inverseMap;


    public FinderSelectionPanel(Frame owner, Collection<ConfNode> matches) {
        super(owner);
        initComponents();

        DefaultListModel listModel = (DefaultListModel) matchList.getModel();

        inverseMap = new HashMap<String, ConfNode>();
        for (ConfNode node : matches) {
            listModel.addElement(node.getInstanceName());
            inverseMap.put(node.getInstanceName(), node);
        }


        matchList.validate();

        setFocusable(true);
        matchList.setSelectedIndex(0);
        matchList.requestFocus();

        matchList.addMouseListener(new MouseAdapter() {

            public void mouseClicked(MouseEvent e) {
                if (e.getClickCount() == 2) {
                    selectEntry();
                }
            }
        });

        KeyAdapter keyAdapter = new KeyAdapter() {

            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_ESCAPE) {
                    selectNull();
                }
                if (e.getKeyCode() == KeyEvent.VK_ENTER) {
                    selectEntry();
                }
            }
        };
        addKeyListener(keyAdapter);
        matchList.addKeyListener(keyAdapter);

        okButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                selectEntry();
            }
        });

        cancelButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                selectNull();
            }
        });
    }


    private void selectNull() {
        selectedNode = null;
        dispose();
    }


    private void selectEntry() {
        selectedNode = inverseMap.get(matchList.getSelectedValue());
        dispose();
    }


    public ConfNode getSelectedNode() {
        return selectedNode;
    }


    private void initComponents() {
        // JFormDesigner - Component initialization - DO NOT MODIFY  //GEN-BEGIN:initComponents
        // Generated using JFormDesigner Open Source Project license - Sphinx-4 (cmusphinx.sourceforge.net/sphinx4/)
        dialogPane = new JPanel();
        contentPanel = new JPanel();
        scrollPane1 = new JScrollPane();
        matchList = new JList(new DefaultListModel());
        buttonBar = new JPanel();
        okButton = new JButton();
        cancelButton = new JButton();

        //======== this ========
        Container contentPane = getContentPane();
        contentPane.setLayout(new BorderLayout());

        //======== dialogPane ========
        {
            dialogPane.setBorder(new MatteBorder(1, 1, 1, 1, Color.black));
            dialogPane.setLayout(new BorderLayout());

            //======== contentPanel ========
            {
                contentPanel.setLayout(new BorderLayout());

                //======== scrollPane1 ========
                {

                    //---- matchList ----
                    matchList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
                    scrollPane1.setViewportView(matchList);
                }
                contentPanel.add(scrollPane1, BorderLayout.CENTER);

                //======== buttonBar ========
                {
                    buttonBar.setBorder(null);
                    buttonBar.setLayout(new GridBagLayout());
                    ((GridBagLayout) buttonBar.getLayout()).columnWidths = new int[]{0, 85, 80, 0};
                    ((GridBagLayout) buttonBar.getLayout()).rowHeights = new int[]{0, 0};
                    ((GridBagLayout) buttonBar.getLayout()).columnWeights = new double[]{1.0, 0.0, 0.0, 1.0E-4};
                    ((GridBagLayout) buttonBar.getLayout()).rowWeights = new double[]{0.0, 1.0E-4};

                    //---- okButton ----
                    okButton.setText("OK");
                    buttonBar.add(okButton, new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0,
                            GridBagConstraints.CENTER, GridBagConstraints.BOTH,
                            new Insets(0, 0, 0, 5), 0, 0));

                    //---- cancelButton ----
                    cancelButton.setText("Cancel");
                    buttonBar.add(cancelButton, new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0,
                            GridBagConstraints.CENTER, GridBagConstraints.BOTH,
                            new Insets(0, 0, 0, 0), 0, 0));
                }
                contentPanel.add(buttonBar, BorderLayout.SOUTH);
            }
            dialogPane.add(contentPanel, BorderLayout.CENTER);
        }
        contentPane.add(dialogPane, BorderLayout.CENTER);
        pack();
        setLocationRelativeTo(getOwner());
        // JFormDesigner - End of component initialization  //GEN-END:initComponents
    }


    // JFormDesigner - Variables declaration - DO NOT MODIFY  //GEN-BEGIN:variables
    // Generated using JFormDesigner Open Source Project license - Sphinx-4 (cmusphinx.sourceforge.net/sphinx4/)
    private JPanel dialogPane;
    private JPanel contentPanel;
    private JScrollPane scrollPane1;
    private JList matchList;
    private JPanel buttonBar;
    private JButton okButton;
    private JButton cancelButton;
    // JFormDesigner - End of variables declaration  //GEN-END:variables
}
