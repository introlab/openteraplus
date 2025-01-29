# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'OpenTeraPlus'
copyright = '2024, Simon Brière, Dominic Létourneau'
author = 'Simon Brière, Dominic Létourneau'
release = '1.3.1'
version = release

html_logo = 'logo/LogoOpenTeraPlus.png'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['myst_parser', 'sphinx_rtd_theme', 'sphinxcontrib.openapi', 'sphinx.ext.autosummary']

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'venv']

locale_dirs = ['locale/']
language = 'fr'  # Default language

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_show_sourcelink = False
html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    'navigation_depth': -1,
    'display_version': True
}

source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}
