/*
 * Scilab ( http://www.scilab.org/ ) - This file is part of Scilab
 * Copyright (C) 2012 - Scilab Enterprises - Calixte DENIZET
 *
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 * http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt
 *
 */

package org.scilab.modules.uiwidget.components;

import java.awt.Color;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.text.DecimalFormat;
import java.util.Map;

import javax.swing.AbstractAction;
import javax.swing.JFormattedTextField;
import javax.swing.JPasswordField;
import javax.swing.JTextField;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import org.scilab.modules.uiwidget.UIComponent;
import org.scilab.modules.uiwidget.UIComponentAnnotation;
import org.scilab.modules.uiwidget.UIWidgetException;
import org.scilab.modules.uiwidget.UIWidgetTools;

public class UITextfield extends UIComponent {

    private JTextField textfield;
    private boolean password;
    private DocumentListener listener;
    private boolean onchangeEnable = true;
    private String onchangeAction;
    private boolean onenterEnable = true;
    private String onenterAction;

    public enum Alignment {
        LEADING (JTextField.LEADING),
        CENTER (JTextField.CENTER),
        LEFT (JTextField.LEFT),
        RIGHT (JTextField.RIGHT),
        TRAILING (JTextField.TRAILING);

        private final int value;

        Alignment(int value) {
            this.value = value;
        }

        public int value() {
            return value;
        }
    }

    public UITextfield(UIComponent parent) throws UIWidgetException {
        super(parent);
    }

    public Object newInstance() {
        textfield = new JTextField();

        return textfield;
    }

    @UIComponentAnnotation(attributes = {"text", "columns", "color", "font", "underline", "bold", "italic", "strike-through", "size", "weight", "password", "format"})
    public Object newInstance(String text, int columns, Color color, String font, boolean underline, boolean bold, boolean italic, boolean strikethrough, double size, UITools.FontWeight weight, boolean password, DecimalFormat format) {
        this.password = password;
        if (format == null) {
            if (password) {
                textfield = new JPasswordField(columns == Integer.MAX_VALUE ? 0 : columns);
            } else {
                textfield = new JTextField(columns == Integer.MAX_VALUE ? 0 : columns);
            }
        } else {
            textfield = new JFormattedTextField(format);
            textfield.setColumns(columns == Integer.MAX_VALUE ? 0 : columns);
        }

        textfield.setText(text);

        if (color != null) {
            textfield.setForeground(color);
        }

        if (font != null && !font.isEmpty()) {
            textfield.setFont(UITools.getFont(null, null, font, size, underline, bold, italic, strikethrough, weight));
        } else {
            textfield.setFont(UITools.getFont(null, null, textfield.getFont(), size, underline, bold, italic, strikethrough, weight));
        }

        return textfield;
    }

    public void setSelectAll(boolean all) {
        if (all) {
            textfield.selectAll();
        } else {
            textfield.select(0, 0);
        }
    }

    public void setHorizontalAlignment(Alignment alignment) {
        textfield.setHorizontalAlignment(alignment.value());
    }

    public String getPassword() {
        if (password) {
            return new String(((JPasswordField) textfield).getPassword());
        } else {
            return textfield.getText();
        }
    }

    public void setEchoChar(String s) {
        if (password && s != null && !s.isEmpty()) {
            ((JPasswordField) textfield).setEchoChar(s.charAt(0));
        }
    }

    public void setString(String s) {
        textfield.setText(s);
    }

    public String getString() {
        return textfield.getText();
    }

    public void removeChangeListener() {
        if (listener != null) {
            textfield.getDocument().removeDocumentListener(listener);
            listener = null;
        }
    }

    public void remove() {
        removeChangeListener();
        textfield.setAction(null);
        super.remove();
    }

    public String getOnchange() {
        return onchangeAction;
    }

    public void setOnchange(final String onchangeAction) {
        if (this.onchangeAction == null) {
            removeChangeListener();
            listener = new DocumentListener() {
                public void insertUpdate(DocumentEvent e) {
                    if (onchangeEnable) {
                        UIWidgetTools.execAction(UITextfield.this, UITextfield.this.onchangeAction);
                    }
                }

                public void removeUpdate(DocumentEvent e) {
                    if (onchangeEnable) {
                        UIWidgetTools.execAction(UITextfield.this, UITextfield.this.onchangeAction);
                    }
                }

                public void changedUpdate(DocumentEvent e) { }
            };
            textfield.getDocument().addDocumentListener(listener);
        }
        this.onchangeAction = onchangeAction;
    }

    public boolean getOnchangeEnable() {
        return onchangeEnable;
    }

    public void setOnchangeEnable(boolean b) {
        onchangeEnable = b;
    }

    public String getOnenter() {
        return onenterAction;
    }

    public void setOnenter(final String onenterAction) {
        if (this.onenterAction == null) {
            textfield.setAction(new AbstractAction() {
                public void actionPerformed(ActionEvent e) {
                    if (onenterEnable) {
                        UIWidgetTools.execAction(UITextfield.this, UITextfield.this.onenterAction);
                    }
                }
            });
        }
        this.onenterAction = onenterAction;
    }

    public boolean getOnenterEnable() {
        return onenterEnable;
    }

    public void setOnenterEnable(boolean b) {
        onenterEnable = b;
    }
}