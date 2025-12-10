#ifndef PDFEXPORTTEMPLATE_H
#define PDFEXPORTTEMPLATE_H

#pragma once

#include <QString>
#include <QColor>

/**
 * @brief Template pour l'export PDF avec styles prédéfinis.
 */
class PdfExportTemplate
{
public:
    /**
     * @brief Constructeur d'un template.
     * @param name Nom du template
     */
    PdfExportTemplate(const QString &name = "Default");
    
    /** @return Nom du template */
    QString name() const { return m_name; }
    
    /** @return Feuille de style CSS */
    QString css() const { return m_css; }
    
    /** @return HTML de l'en-tête */
    QString headerHtml() const { return m_headerHtml; }
    
    /** @return HTML du pied de page */
    QString footerHtml() const { return m_footerHtml; }
    
    /** @brief Définit le nom du template */
    void setName(const QString &name) { m_name = name; }
    
    /** @brief Définit la feuille de style CSS */
    void setCss(const QString &css) { m_css = css; }
    
    /** @brief Définit le HTML de l'en-tête */
    void setHeaderHtml(const QString &html) { m_headerHtml = html; }
    
    /** @brief Définit le HTML du pied de page */
    void setFooterHtml(const QString &html) { m_footerHtml = html; }
    
    // Templates prédéfinis
    
    /** @return Template par défaut */
    static PdfExportTemplate defaultTemplate();
    
    /** @return Template minimaliste */
    static PdfExportTemplate minimalTemplate();
    
    /** @return Template détaillé */
    static PdfExportTemplate detailedTemplate();
    
    /** @return Template coloré */
    static PdfExportTemplate colorfulTemplate();
    
    /** @return Liste de tous les templates disponibles */
    static QList<PdfExportTemplate> availableTemplates();
    
private:
    QString m_name;
    QString m_css;
    QString m_headerHtml;
    QString m_footerHtml;
};

#endif // PDFEXPORTTEMPLATE_H
