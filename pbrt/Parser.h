// ======================================================================== //
// Copyright 2015-2018 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "pbrt/Scene.h"
#include "Lexer.h"
// std
#include <stack>

namespace pbrt_parser {

  /*! the class that implements PBRT's "current transformation matrix
      (CTM)" stack */
  struct CTM : public Transforms {
    void reset()
    {
      startActive = endActive = true;
      atStart = atEnd = affine3f(ospcommon::one);
    }
    bool startActive { true };
    bool endActive   { true };
    
    /*! pbrt's "CTM" (current transformation matrix) handling */
    std::stack<Transforms> stack;
  };

  
  /*! parser object that holds persistent state about the parsing
    state (e.g., file paths, named objects, etc), even if they are
    split over multiple files. To parse different scenes, use
    different instances of this class. */
  struct PBRT_PARSER_INTERFACE Parser {
    /*! constructor */
    Parser(bool dbg, const std::string &basePath="");

    /*! parse given file, and add it to the scene we hold */
    void parse(const FileName &fn);

    /*! parse everything in WorldBegin/WorldEnd */
    void parseWorld();
    /*! parse everything in the root scene file */
    void parseScene();
      
    void pushAttributes();
    void popAttributes();
      
    /*! try parsing this token as some sort of transform token, and
      return true if successful, false if not recognized  */
    bool parseTransforms(TokenHandle token);

    void pushTransform();
    void popTransform();

    vec3f parseVec3f();
    float parseFloat();
    affine3f parseMatrix();


    std::map<std::string,std::shared_ptr<Object> >   namedObjects;

    inline std::shared_ptr<Param> parseParam(std::string &name);
    void parseParams(std::map<std::string, std::shared_ptr<Param> > &params);

    /*! return the scene we have parsed */
    std::shared_ptr<Scene> getScene() { return scene; }
    std::shared_ptr<Texture> getTexture(const std::string &name);
  private:
    //! stack of parent files' token streams
    std::stack<std::shared_ptr<Lexer>> tokenizerStack;
    std::deque<TokenHandle> peekQueue;
    
    //! token stream of currently open file
    std::shared_ptr<Lexer> tokens;
    
    /*! get the next token to process (either from current file, or
      parent file(s) if current file is EOL!); return NULL if
      complete end of input */
    TokenHandle next();

    /*! peek ahead by N tokens, (either from current file, or
      parent file(s) if current file is EOL!); return NULL if
      complete end of input */
    TokenHandle peek(int ahead=0);

    // add additional transform to current transform
    void addTransform(const affine3f &xfm)
    {
      if (ctm.startActive) ctm.atStart *= xfm;
      if (ctm.endActive)   ctm.atEnd   *= xfm;
    }
    void setTransform(const affine3f &xfm)
    {
      if (ctm.startActive) ctm.atStart = xfm;
      if (ctm.endActive) ctm.atEnd = xfm;
    }

    std::stack<std::shared_ptr<Material> >   materialStack;
    std::stack<std::shared_ptr<Attributes> > attributesStack;
    std::stack<std::shared_ptr<Object> >     objectStack;

    CTM ctm;
    // Transforms              getCurrentXfm() { return transformStack.top(); }
    std::shared_ptr<Object> getCurrentObject();
    std::shared_ptr<Object> findNamedObject(const std::string &name, bool createIfNotExist=false);

    // emit debug status messages...
    bool dbg;
    const std::string basePath;
    FileName rootNamePath;
    std::shared_ptr<Scene>    scene;
    std::shared_ptr<Object>   currentObject;
    std::shared_ptr<Material> currentMaterial;
    
    /*! tracks the location of the last token gotten through next() (for debugging) */
    Loc lastLoc;
    
  };

  PBRT_PARSER_INTERFACE void parsePLY(const std::string &fileName,
                                      std::vector<vec3f> &v,
                                      std::vector<vec3f> &n,
                                      std::vector<vec3i> &idx);
}
