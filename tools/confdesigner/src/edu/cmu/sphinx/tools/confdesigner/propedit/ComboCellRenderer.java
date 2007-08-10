package edu.cmu.sphinx.tools.confdesigner.propedit;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellRenderer;
import java.awt.*;

class ComboTableCellRenderer implements ListCellRenderer, TableCellRenderer {

    DefaultListCellRenderer listRenderer = new DefaultListCellRenderer();

    DefaultTableCellRenderer tableRenderer = new DefaultTableCellRenderer();


    private void configureRenderer(JLabel renderer, Object value) {
        if ((value != null) && (value instanceof Color)) {
            renderer.setText(value.toString());
            renderer.setBackground((Color) value);
        } else {
            renderer.setText(value.toString());
        }
    }


    public Component getListCellRendererComponent(JList list, Object value, int index,
                                                  boolean isSelected, boolean cellHasFocus) {
        listRenderer = (DefaultListCellRenderer) listRenderer.getListCellRendererComponent(list, value,
                index, isSelected, cellHasFocus);
        configureRenderer(listRenderer, value);
        return listRenderer;
    }


    public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected,
                                                   boolean hasFocus, int row, int column) {
        tableRenderer = (DefaultTableCellRenderer) tableRenderer.getTableCellRendererComponent(table,
                value, isSelected, hasFocus, row, column);
        configureRenderer(tableRenderer, value);
        return tableRenderer;
    }
}