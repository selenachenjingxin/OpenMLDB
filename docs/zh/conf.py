# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

#import maisie_sphinx_theme

# -- Project information -----------------------------------------------------

project = 'OpenMLDB'
copyright = '2022, OpenMLDB'
author = 'OpenMLDB'

# The full version, including alpha/beta/rc tags
version = 'main'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
'myst_parser',
'sphinx_multiversion',
'sphinx_copybutton',
'sphinx.ext.autosectionlabel',
]

autosectionlabel_prefix_document = True

myst_heading_anchors = 6

myst_enable_extensions = [
    "amsmath",
    "colon_fence",
    "deflist",
    "dollarmath",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",
    "replacements",
    "smartquotes",
    "strikethrough",
    "substitution",
    "tasklist",
]


# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']


#smv_latest_version = 'v0.4'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
#language = 'zh_CN'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
#html_theme = 'alabaster'
#html_theme = 'furo'
html_theme = 'sphinx_book_theme'

html_theme_options = {
#    "sidebar_hide_name": True,
    "logo_only": True,
    "repository_url": "https://github.com/4paradigm/OpenMLDB/tree/main/docs/zh",
    "use_repository_button": True,
}


#html_sidebars = {
#    '**': [
#	"sidebar/brand.html",
#	"sidebar/search.html",
#	"sidebar/scroll-start.html",
#	"sidebar/navigation.html",
#	"sidebar/scroll-end.html",
#    "versioning.html",
#    ],
#}


html_sidebars = {
    '**': [
    "sidebar-logo.html",
    "search-field.html",
    "sbt-sidebar-nav.html",
    "versioning.html",
    ],
}


html_last_updated_fmt = "%c"
master_doc = "index"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = []

html_logo = "about/images/openmldb_logo.png"


# ================================== #
# sphinx multiversion configuration  #
# ================================== #

# Whitelist pattern for tags (set to None to ignore all tags)
# no tags included
smv_tag_whitelist = None

# Whitelist pattern for branches (set to None to ignore all branches)
# include branch that is main or v{X}.{Y}
smv_branch_whitelist = r"^(main|quickstartOpti)$"

# allow remote origin or upstream
smv_remote_whitelist = r"^(origin|upstream)$"
